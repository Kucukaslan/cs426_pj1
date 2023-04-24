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
    int count = 0;
    if (argc != 3)
    {
        printf("Usage: %s input_file output_file \n", argv[0]);
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
            // printf("%d %d, min, max: %d %d\n", bucket, key, min, max);
            if (bucket < min)
                min = bucket;
            if (bucket > max)
                max = bucket;
        }
        printf("%d cnt %d, min, max: %d %d\n", rank, count, min, max);

        // broadcast the range of bucket indices
        MPI_Bcast(&min, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&max, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int *buckets = (int *)malloc(count * sizeof(int));
        int *keys = (int *)malloc(count * sizeof(int));

        int i = 0;
        rewind(fd);
        while (fscanf(fd, "%d %d", &bucket, &key) != EOF)
        {
            buckets[i] = bucket;
            keys[i] = key;
            // printf("%d: content of file: %d %d or %d %d\n", rank, bucket, key, buckets[i], keys[i]);
            i++;
        }

        // create sendcounts and displs so that each process gets the same number of elements
        int *sendcounts = (int *)malloc(size * sizeof(int));
        int *displs = (int *)malloc(size * sizeof(int));

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
        // print sendcounts and displs
        for (int i = 0; i < size; i++)
        {
            printf("%d: sendcounts[%d] = %d, displs[%d] = %d\n", rank, i, sendcounts[i], i, displs[i]);
        }

        // broadcast the sendcounts and displs to all processes
        MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

        int *recvB = (int *)malloc(count * sizeof(int));
        int *recvK = (int *)malloc(count * sizeof(int));

        MPI_Scatterv(buckets, sendcounts, displs, MPI_INT, recvB, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(keys, sendcounts, displs, MPI_INT, recvK, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

        for (int i = 0; i < sendcounts[rank]; i++)
        {
            printf("%d:%d bucket key %d %d\n", rank, i, recvB[i], recvK[i]);
        }

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

        // FILE *outfd = fopen(argv[2], "w");
        float avg;
        for (int i = 0; i < range - 1; i++)
        {
            avg = (float)sums[i] / counts[i];
            printf("%d:%d %d/%d = %f\n", rank, i + min, sums[i], counts[i], avg);
            // fprintf(outfd, "%.1f\n", avg);
        }

        // reduce the sums and counts
        int *sumsAll = (int *)malloc(range * sizeof(int));
        int *countsAll = (int *)malloc(range * sizeof(int));
        MPI_Reduce(sums, sumsAll, range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(counts, countsAll, range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // print the averages
        FILE *outfd = fopen(argv[2], "w");
        for (int i = 0; i < range - 1; i++)
        {
            avg = (float)sumsAll[i] / countsAll[i];
            printf("%d:%d %d/%d = %f\n", rank, i + min, sumsAll[i], countsAll[i], avg);
            fprintf(outfd, "%.1f\n", avg);
        }
        avg = (float)sumsAll[range - 1] / countsAll[range - 1];
        printf("%d:%d %d/%d = %f\n", rank, range - 1 + min, sumsAll[range - 1], countsAll[range - 1], avg);
        fprintf(outfd, "%.1f", avg);
        fclose(outfd);

        free(buckets);
        free(keys);
        free(recvB);
        free(recvK);
    }
    else // other processes
    {
        // receive the range of bucket indices
        MPI_Bcast(&min, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&max, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int *sendcounts = (int *)malloc(size * sizeof(int));
        int *displs = (int *)malloc(size * sizeof(int));

        MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);

        int *recvB = (int *)malloc(count * sizeof(int));
        int *recvK = (int *)malloc(count * sizeof(int));

        MPI_Scatterv(NULL, sendcounts, displs, MPI_INT, recvB, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(NULL, sendcounts, displs, MPI_INT, recvK, sendcounts[rank], MPI_INT, 0, MPI_COMM_WORLD);

        for (int i = 0; i < sendcounts[rank]; i++)
        {
            printf("%d:%d bucket key %d %d\n", rank, i, recvB[i], recvK[i]);
        }

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

        float avg;
        for (int i = 0; i < range - 1; i++)
        {
            avg = (float)sums[i] / counts[i];
            printf("%d:%d %d/%d = %f\n", rank, i + min, sums[i], counts[i], avg);
            // fprintf(outfd, "%.1f\n", avg);
        }

        // reduce the sums and counts
        // reduce the sums and counts
        int *sumsAll = (int *)malloc(range * sizeof(int));
        int *countsAll = (int *)malloc(range * sizeof(int));
        MPI_Reduce(sums, sumsAll, range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(counts, countsAll, range, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        free(recvB);
        free(recvK);
    }
    // FILE *outfd = fopen(argv[2], "w");
    // float avg;
    // for (int i = 0; i < count; i++)
    // {
    //     avg = (float)sums[i] / counts[i];
    //     // printf("%d/%d = %f\n", sums[i], counts[i], avg);
    //     fprintf(outfd, "%.1f\n", avg);
    // }
    // fclose(fd);
    // fclose(outfd);
    MPI_Finalize();
    return 0;
}