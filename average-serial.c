#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *fd;
    int tmp;

    if (argc != 2)
    {
        printf("Usage: %s filename \n", argv[0]);
        return 1;
    }
  
    fd = fopen(argv[1], "r");

    int count = 0;
    fscanf (fd, "%d", &count);

    int sum = 0;
    for(int i = 0; i < count; i++) {
        fscanf (fd, "%d", &tmp);
        sum += tmp;
        // printf("%d: data = %d, sum= %d\n",i, tmp, sum);  
    } 
    printf("%f\n", (float)sum/count);
    fclose (fd);

    return 0;
}