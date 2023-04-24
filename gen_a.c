#include <stdio.h>
#include <stdlib.h>

// generate a file with random numbers in [10,100]
// usage: gen_a <filename> <number of numbers>
// the file will contain <number of numbers> numbers
// the first line of the file will contain <number of numbers>
// the rest of the file will contain random numbers in [10,100]
int main(int argc, char **argv)
{
    FILE *fd;
    int n;
    int i;
    int r;
    if (argc != 3)
    {
        printf("Usage: %s filename number_of_numbers\n", argv[0]);
        return 1;
    }
    fd = fopen(argv[1], "w");
    if (fd == NULL)
    {
        printf("Error: cannot open file %s \n", argv[1]);
        return 1;
    }
    n = atoi(argv[2]);
    fprintf(fd, "%d\n", n);
    for (i = 0; i < n - 1; i++)
    {
        r = 10 + rand() % 90;
        fprintf(fd, "%d\n", r);
    }
    r = 10 + rand() % 90;
    fprintf(fd, "%d", r);
    fclose(fd);
    return 0;
}