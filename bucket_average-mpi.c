#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Status s;
    int size, rank;
    FILE *fd;
    int bucket, key;
    int min, max;
    int *buckets;
    int *keys;
    int count = 0;
    if (argc != 3)
    {
        // printf("Usage: %s input_file output_file \n", argv[0]);
        return 1;
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) // Master process
    {
        fd = fopen(argv[1], "r");
        if (fd == NULL)
        {
            printf("Error: cannot open file %s \n", argv[1]);
            return 1;
        }

        // find the range of bucket indices
        if (fscanf(fd, "%d", &bucket) == EOF)
        {
            printf("Error: empty file %s \n", argv[1]);
            return 1;
        }
        min = bucket;
        max = bucket;
        count = 0;
        rewind(fd);
        while (fscanf(fd, "%d %d", &bucket, &key) != EOF)
        {
            count += 1;
            //// printf("%d %d, min, max: %d %d\n", bucket, key, min, max);
            if (bucket < min)
                min = bucket;
            if (bucket > max)
                max = bucket;
        }
        // printf("%d cnt %d, min, max: %d %d\n", rank, count, min, max);
    }
    // broadcast the range of bucket indices
    MPI_Bcast(&min, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // create sendcounts and displs so that each process gets the same number of elements
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));

    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
            sendcounts[i] = 0;
            displs[i] = 0;
        }
        for (int i = 0; i < count; i++)
        {
            sendcounts[i % size]++;
        }

        displs[0] = 0;
        for (int i = 1; i < size; i++)
        {
            displs[i] = displs[i - 1] + sendcounts[i - 1];
        }
    }
    buckets = (int *)malloc(count * sizeof(int));
    keys = (int *)malloc(count * sizeof(int));

    MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        int i = 0;
        rewind(fd);
        while (fscanf(fd, "%d %d", &bucket, &key) != EOF)
        {
            buckets[i] = bucket;
            keys[i] = key;
            //// printf("%d: content of file: %d %d or %d %d\n", rank, bucket, key, buckets[i], keys[i]);
            i++;
        }
        fclose(fd);
    }

    int *recvB = (int *)malloc(sendcounts[rank] * sizeof(int));
    int *recvK = (int *)malloc(sendcounts[rank] * sizeof(int));

    MPI_Scatterv(buckets, sendcounts, displs, MPI_INT, recvB, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(keys, sendcounts, displs, MPI_INT, recvK, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

    // for (int i = 0; i < sendcounts[rank]; i++)
    // {
    //     // printf("%d:%d bucket key %d %d\n", rank, i, recvB[i], recvK[i]);
    // }

    int range = max - min + 1;
    int *sums = (int *)malloc(range * sizeof(int));
    int *counts = (int *)malloc(range * sizeof(int));
    // initialize sums and counts
    for (int i = 0; i < range; i++)
    {
        sums[i] = 0;
        counts[i] = 0;
    }
    for (int i = 0; i < sendcounts[rank]; i++)
    {
        sums[recvB[i] - min] += recvK[i];
        counts[recvB[i] - min]++;
    }

    // local sums and counts
    // FILE *outfd = fopen(argv[2], "w");
    // float avg;
    // for (int i = 0; i < range - 1; i++)
    // {
    //     avg = (float)sums[i] / counts[i];
    //    // printf("%d:%d %d/%d = %f\n", rank, i + min, sums[i], counts[i], avg);
    //     // fprintf(outfd, "%.1f\n", avg);
    // }

    // reduce the sums and counts
    int *sumsAll = (int *)malloc(range * sizeof(int));
    int *countsAll = (int *)malloc(range * sizeof(int));
    // initialize sums and counts
    for (int i = 0; i < range; i++)
    {
        sumsAll[i] = 0;
        countsAll[i] = 0;
    }
    MPI_Reduce(sums, sumsAll, range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(counts, countsAll, range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        // print the averages
        FILE *outfd = fopen(argv[2], "wt+");
        float avg;
        for (int i = 0; i < range - 1; i++)
        {
            if (countsAll[i] == 0)
            {
                printf("%d there is no element in bucket %d, sum is 0, count is 0, average is 0\n", rank, i + min);
                fprintf(outfd, "0.0\n");
                continue;
            }
            avg = (float)sumsAll[i] / countsAll[i];
            // printf("%d:%d %d/%d = %f\n", rank, i + min, sumsAll[i], countsAll[i], avg);
            fprintf(outfd, "%.1f\n", avg);
        }
        avg = (float)sumsAll[range - 1] / countsAll[range - 1];
        // printf("%d:%d %d/%d = %f\n", rank, range - 1 + min, sumsAll[range - 1], countsAll[range - 1], avg);
        fprintf(outfd, "%.1f", avg);
        fclose(outfd);
    }

    free(buckets);
    free(keys);
    free(recvB);
    free(recvK);
    free(sums);
    free(counts);
    free(sumsAll);
    free(countsAll);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}