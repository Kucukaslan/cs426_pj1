#include <stdlib.h>
#include <stdio.h>

// generate bucket indices and random numbers
// usage: gen_b <filename> <number of numbers>
// the file will contain <number of numbers> pairs
int main(int argc, char **argv)
{
    FILE *fd;
    int n;
    int i;
    int r, b;
    if (argc != 3)
    {
        printf("Usage: %s filename number_of_numbers", argv[0]);
        return 1;
    }

    fd = fopen(argv[1], "w");
    if (fd == NULL)
    {
        printf("Error: cannot open file %s ", argv[1]);
        return 1;
    }
    n = atoi(argv[2]);
    for (int i = 0; i < n - 1; i++)
    {
        r = 10 + rand() % 90;
        b = 3 + rand() % 8;
        fprintf(fd, "%d %d\n", b, r);
    }
    r = 10 + rand() % 90;
    b = 3 + rand() % 8;
    fprintf(fd, "%d %d", b, r);
    fclose(fd);
    return 0;
}
