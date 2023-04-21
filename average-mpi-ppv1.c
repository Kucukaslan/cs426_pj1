#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   MPI_Status s;
   int size, rank, i, j;

   if (argc != 2)
   {
      printf("Usage: %s filename \n", argv[0]);
      return 1;
   }

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   size--; // 1 process is the master

   if (rank == 0) // Master process
   {
      FILE *fd;
      int tmp;

      fd = fopen(argv[1], "r");

      int count = 0;
      int sum = 0;
      fscanf(fd, "%d", &count);

      // scan the numbers into the array
      int *numbers = (int *)malloc(count * sizeof(int));
      for (int i = 0; i < count; i++)
      {
         fscanf(fd, "%d", &numbers[i]);
      }
      fclose(fd);

      // send numbers to the other processes
      for (int i = 1; i < count; i++)
      {
         int dst = 1 + (i % (size)); // 1 to size-1 numbers send w.r.t. modulus
         printf("Sending %d to %d\n", numbers[1 + i % size], dst);
         MPI_Send((void *)&numbers[1 + i % size], 1, MPI_INT, dst, 0xACE5, MPI_COMM_WORLD);
         printf("Sent %d to %d\n", numbers[1 + i % size], dst);
      }

      // MPI_Send((void *)&tmp, 1, MPI_INT, dst, 0xACE5, MPI_COMM_WORLD);

      printf("Receiving local sums . . .\n");
      int local_sum = 0;
      for (i = 1; i < size; i++)
      {
         MPI_Recv((void *)&local_sum, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD, &s);
         printf("local sum: [%d] sent %d\n", i, local_sum);
         sum += local_sum;
      }
      printf("The average is %d", sum / count);
   }
   else
   {
      printf("%d Receiving data . . .\n", rank);
      // compute expected number of numbers
      int expected = 2; //(count / size) + (rank < (count % size) ? 1 : 0);
      int local_sum = 0;
      // receive the numbers from the master process and calculate the local sum
      for (i = expected - 1; expected >= 0; i++)
      {
         MPI_Recv((void *)&j, 1, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, &s);
         printf("%d Received %d from master\n", rank, j);
         local_sum += j;
      }

      // send the local sum to the master process
      printf("%d Sending local sum %d to master\n", rank, local_sum);
      MPI_Send((void *)&local_sum, 1, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD);
      printf("%d Sent local sum %d to master\n", rank, local_sum);
   }

   MPI_Finalize();
   return 0;
}
