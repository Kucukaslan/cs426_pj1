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
        count = 1;
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

        int *buckets = (int *)malloc(count * sizeof(int));
        int *keys = (int *)malloc(count * sizeof(int));

        int i = 0;
        rewind(fd);
        while (fscanf(fd, "%d %d", &bucket, &key) != EOF)
        {
            printf("%d: content of file: %d %d\n", rank, bucket, key);
            buckets[i] = bucket;
            keys[i] = key;
        }

        // create sendcounts and displs so that each process gets the same number of elements
        int *sendcounts = (int *)malloc(size * sizeof(int));
        int *displs = (int *)malloc(size * sizeof(int));

        for (int i = 0; i < size; i++)
        {
            sendcounts[i] = count / size;
            displs[i] = i * count / size;
        }

        int *recvB = (int *)malloc(count * sizeof(int));
        int *recvK = (int *)malloc(count * sizeof(int));

        MPI_Scatterv(buckets, sendcounts, displs, MPI_INT, recvB, count / size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(keys, sendcounts, displs, MPI_INT, recvK, count / size, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);

        for (int i = 0; i < count / size; i++)
        {
            printf("%d:%d bucket key %d %d\n", rank, i, recvB[i], recvK[i]);
        }
        free(buckets);
        free(keys);
        free(recvB);
        free(recvK);
    }
    else
    {
        int *recvB = (int *)malloc(count * sizeof(int));
        int recvcntB;
        int *recvK = (int *)malloc(count * sizeof(int));
        int recvcntK;

        int *sendcounts = (int *)malloc(size * sizeof(int));
        int *displs = (int *)malloc(size * sizeof(int));

        for (int i = 0; i < size; i++)
        {
            sendcounts[i] = count / size;
            displs[i] = i * count / size;
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Scatterv(NULL, sendcounts, displs, MPI_INT, recvB, count / size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(NULL, sendcounts, displs, MPI_INT, recvK, count / size, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < 2; i++)
        {
            printf("%d:%d %d %d\n", rank, i, recvB[i], recvK[i]);
        }
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

    return 0;
}