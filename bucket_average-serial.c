#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fd;
    int bucket, key;
    int min, max;

    if (argc != 3)
    {
        printf("Usage: %s input_file output_file \n", argv[0]);
        return 1;
    }

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

    rewind(fd);
    while (fscanf(fd, "%d %d", &bucket, &key) != EOF)
    {
        // printf("%d %d, min, max: %d %d\n", bucket, key, min, max);
        if (bucket < min)
            min = bucket;
        if (bucket > max)
            max = bucket;
    }

    int range = max - min + 1;
    int *sums = (int *)malloc(range * sizeof(int));
    int *counts = (int *)malloc(range * sizeof(int));
    rewind(fd);

    while (fscanf(fd, "%d %d", &bucket, &key) != EOF)
    {
        sums[bucket - min] += key;
        counts[bucket - min]++;
    }

    FILE *outfd = fopen(argv[2], "w");
    float avg;
    for (int i = 0; i < range - 1; i++)
    {
        avg = (float)sums[i] / counts[i];
        // printf("%d/%d = %f\n", sums[i], counts[i], avg);
        fprintf(outfd, "%.1f\n", avg);
    }
    // last line
    avg = (float)sums[range - 1] / counts[range - 1];
    fprintf(outfd, "%.1f", avg);

    fclose(fd);
    fclose(outfd);

    free(sums);
    free(counts);

    return 0;
}