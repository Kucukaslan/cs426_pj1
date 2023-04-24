#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    // printf("we are alive %d", 0);
    MPI_Status s;
    int size, rank, i, j;

    if (argc != 2)
    {
        // printf("Usage: %s filename \n", argv[0]);
        return 1;
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // printf("%d: size=%d\n", rank, size);

    if (rank == 0) // Master process
    {
        FILE *fd;

        fd = fopen(argv[1], "r");

        int tot_count = 0;
        int sum = 0;
        fscanf(fd, "%d", &tot_count);

        // send "tot_count/size" numbers to the other processes
        // first notify the other processes that they will count=tot_count/size numbers
        int count = tot_count / size;
        for (int i = 1; i < size; i++)
        {
            // printf("Sending %d to %d\n", count, i);
            MPI_Send(&count, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
            // send the numbers

            int *numbers = (int *)malloc(count * sizeof(int));
            for (int i = 0; i < count; i++)
            {
                fscanf(fd, "%d", &numbers[i]);
            }
            // fclose(fd);

            MPI_Send(numbers, count, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
            // printf("Sent %d to %d of %d\n", numbers[0], i, size);
        }

        // int remaining = tot_count - count * (size - 1);
        // int *numbers = (int *)malloc(count * sizeof(int));
        // for (int i = 0; i < remaining; i++)
        // {
        //     fscanf(fd, "%d", &numbers[i]);
        // }
        // fclose(fd);
        // printf("%d: call reduce\n", rank);
        // MPI_Allreduce(numbers, &sum, remaining, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        // int local_sum = 0;
        // for (int i = 0; i < remaining; i++)
        // {
        //     local_sum += numbers[i];
        //     // printf("%d: received %d, local sum %d\n", rank, numbers[i], local_sum);
        // }
        // printf("%d: all reduce sum %d; loc sum: %d\n", rank, sum, local_sum);
        // compute the sum of remaining numbers
        int tmp = 0;
        for (int i = count * (size - 1); i < tot_count; i++)
        {
            fscanf(fd, "%d", &tmp);
            sum += tmp;
        }
        tmp = sum;
        fclose(fd);
        MPI_Allreduce(&tmp, &sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        // // printf("Receiving local sums . . .%d\n", sum);
        // int local_sum = 0;
        // for (i = 1; i < size; i++)
        // {
        //     MPI_Recv((void *)&local_sum, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD, &s);
        //     // printf("local sum: [%d] sent %d\n", i, local_sum);
        //     sum += local_sum;
        // }
        // printf("The average is %f", 1.0 * sum / tot_count);
        printf("%f\n", 1.0 * sum / tot_count);
    }
    else
    {
        int count;
        int local_sum = 0;
        int sum;

        // receive the numbers from the master process and calculate the local sum
        // printf("%d Receiving data . . .\n", rank);

        MPI_Recv((void *)&count, 1, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, &s);
        // printf("%d Receiving %d numbers from master\n", rank, count);
        int *numbers = (int *)malloc(count * sizeof(int));
        // int *out = (int *)malloc(count * sizeof(int));
        MPI_Recv(numbers, count, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, &s);
        // printf("%d Received %d numbers from master\n", rank, count);
        // printf("%d first number %d\n", rank, numbers[0]);

        for (int i = 0; i < count; i++)
        {
            local_sum += numbers[i];
            // printf("%d: received %d, local sum %d; \n", rank, numbers[i], local_sum);
        }
        MPI_Allreduce(&local_sum, &sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        // printf("%d: all redmuce sum %d; loc sum: %d\n", rank, sum, local_sum);

        // // send the local sum to the master process
        // // printf("%d Sending local sum %d to master\n", rank, local_sum);
        // MPI_Send((void *)&local_sum, 1, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD);
        // // printf("%d Sent local sum %d to master\n", rank, local_sum);
    }

    MPI_Finalize();
    return 0;
}
