#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
/*Create a function that creates n child processes */
int *delegator(int n, int upper, int lower)
{
    for (int i = 0; i < n; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            /* Child process */
            printf("Child process %d\n", i);
            exit(0);
        }
        else if (pid < 0)
        {
            /* Error */
            printf("Error");
            exit(1);
        }
    }
    return 0;
}

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
            /*printf("lower: %d\n", lower);*/
        }
        if (strcmp(argv[i], "-u") == 0)
        {
            upper = atoi(argv[i + 1]);
            /*printf("upper: %d\n", upper);*/
        }
        if (strcmp(argv[i], "-e") == 0)
        {
            equal = true;
            /*printf("equal: %d\n", equal);*/
        }
        if (strcmp(argv[i], "-r") == 0)
        {
            random = true;
            /*printf("random: %d\n", random);*/
        }
        if (strcmp(argv[i], "-n") == 0)
        {
            n = atoi(argv[i + 1]);
            /*printf("n: %d\n", n);*/
        }
    }
    /* Create n child processes */
    for (int j = 0; j < n; j++)
    {
        int pid = fork();
        if (pid == 0)
        {
            /* Child process */
            printf("Delegator process %d\n", j);
            /* Each delegator takes 1/n range from upper and lower bounds */
            int upper_bound = upper - (upper - lower) / n * j;
            int lower_bound = upper - (upper - lower) / n * (j + 1);
            printf("upper_bound: %d\n", upper_bound);
            printf("lower_bound: %d\n", lower_bound);
            delegator(n, upper_bound, lower_bound);
            exit(0);
        }
        else if (pid < 0)
        {
            /* Error */
            printf("Error");
            exit(1);
        }
    }
    return 0;
};