#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{
    /* Check flags */
    int lower = 0;
    int upper = 0;
    bool equal = false;
    bool random = false;
    int n = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            lower = atoi(argv[i + 1]);
            printf("lower: %d", lower);
        }
        if (strcmp(argv[i], "-u") == 0)
        {
            upper = atoi(argv[i + 1]);
            printf("upper: %d", upper);
        }
        if (strcmp(argv[i], "-e") == 0)
        {
            equal = true;
            printf("equal: %d", equal);
        }
        if (strcmp(argv[i], "-r") == 0)
        {
            random = true;
            printf("random: %d", random);
        }
        if (strcmp(argv[i], "-n") == 0)
        {
            n = atoi(argv[i + 1]);
            printf("n: %d", n);
        }
    }
    return 0;
}