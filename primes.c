#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
// Create functions to find primes for a given subrange
// --- Professor Provided functions to check if number is prime ---
int prime1(int n)
{
    int i;
    if (n == 1 || n == 0)
        return 0;
    for (i = 2; i < n; i++)
        if (n % i == 0)
            return 0;
    return 1;
}

int prime2(int n)
{
    int i = 0, limitup = 0;
    limitup = (int)(sqrt((float)n));
    if (n == 1 || n == 0)
        return 0;
    for (i = 2; i <= limitup; i++)
        if (n % i == 0)
            return 0;
    return 1;
}
// --- Professor Provided functions to check if number is prime ---

// --- Finding primes for a given subrange ---

/* Create a struct to hold the primes and the size of the array */
typedef struct
{
    int *primes; // array of primes in a given subrange
    int size;    // size of the array
} primes_in_subrange;

/* Create functions to find primes for a given subrange */
primes_in_subrange find_primes1(int lower, int upper)
{
    int *primes = malloc(sizeof(int) * (upper - lower));
    int size = 0;
    for (int i = lower; i < upper + 1; i++)
    {
        if (prime1(i))
        {
            primes[size] = i;
            size++;
        }
    }
    primes_in_subrange result = {primes, size};
    return result;
}

primes_in_subrange find_primes2(int lower, int upper)
{
    int *primes = malloc(sizeof(int) * (upper - lower));
    int size = 0;
    for (int i = lower; i < upper + 1; i++)
    {
        if (prime2(i))
        {
            primes[size] = i;
            size++;
        }
    }
    primes_in_subrange result = {primes, size};
    return result;
}

// Function to generate n random intervals in the range [lower, upper]
// The intervals will look something like this for n = 4:
// [lower, x1], [x1 + 1, x2], [x2 + 1, x3], [x3 + 1, upper]
// Where x1, x2, x3 are random numbers in the range [lower, upper]
// that divide the range [lower, upper] into n equal subranges
int *generate_random_intervals(int lower, int upper, int n) {
    srand(time(0));

    // Create 2 * n "points" randomly in the range [lower, upper]
    // Ensuring that the first point == lower and the (n + 1)th point == upper
    int* points = malloc(sizeof(int) * 2 * n);

    points[0] = lower;
    points[(2 * n) - 1] = upper;

    // If upper - lower + 1 < 2 * n, then we can't create n intervals
    // In that case, create as many intervals as possible and fill the rest with -1
    if (upper - lower + 1 < 2 * n) {
        for (int i = 1; i < (2 * n) - 1; i += 2) {
            points[i] = lower + i / 2;
            points[i + 1] = lower + i / 2 + 1;
        }
        return points;
    }

    // If lower == upper, then we can just return the points
    if (lower == upper) {
        return points;
    }

    for (int i = 1; i < (2 * n) - 1; i += 2) {
        // Create a random point such that:
        // 1. The point is greater than the previous point
        // 2. The point is less than the upper bound

        int lower_bound = points[i - 1] + 1;
        int upper_bound = upper - (2 * n - i - 1);

        points[i] = lower_bound + rand() % (upper_bound - lower_bound + 1);
        // Now, create a point that is this point + 1
        // This ensures that the points are not equal
        points[i + 1] = points[i] + 1;
    }

    return points;
}

/*Create a function that creates n child processes */
int *delegator(int j, int n, int upper, int lower, char **fifonames)
{
    int num_primes = 0;
    bool random = true;
    bool flag = true; // if flag is true, then we assign function 1 to the first child process and flip
    if (n % 2 != 0 && j % 2 == 0)
    {
        flag = false; // if n is odd and j is even, then we assign function 2 to the first child process and flip
    }
    for (int i = 0; i < n; i++)
    {
        int fd_r = open(fifonames[i + j * n], O_RDONLY | O_NONBLOCK);
        int pid = fork();
        if (pid == 0)
        {
            /* Child process */
            /*printf("Child process %d\n", i);*/
            /*printf("Named pipe: %s\n", fifonames[i + j * n]);*/

            /* Open named pipe */
            int fd = open(fifonames[i + j * n], O_WRONLY | O_NONBLOCK);
            if (fd == -1)
            {
                perror("Error opening named pipe\n");
                exit(1);
            }
            int sublower = 0;
            int subupper = 0;
            if (!random) {
            sublower = lower + (upper - lower) / n * i;
            subupper = lower + (upper - lower) / n * (i + 1) - 1;
            if (i == n - 1)
            {
                subupper = upper;
            }
            }
            else {
                int *points = generate_random_intervals(lower, upper, n);
                sublower = points[2 * i];
                subupper = points[2 * i + 1];
            }
            if (flag)
            {
                // split the range into n subranges and find the primes in each subrange
                // printf("Finding primes in range %d to %d using function 1\n", sublower, subupper);
                primes_in_subrange result = find_primes1(sublower, subupper);
                for (int k = 0; k < result.size; k++)
                {
                    // printf("%d ", result.primes[k]);
                    int bytes_written = write(fd, &result.primes[k], sizeof(int));
                    if (bytes_written == -1)
                    {
                        perror("write");
                        close(fd);
                        return 1;
                    }
                }
                printf("\n");
                // Close the named pipe
                close(fd);
            }
            else
            {
                // split the range into n subranges and find the primes in each subrange
                // printf("Finding primes in range %d to %d using function 2\n", sublower, subupper);
                primes_in_subrange result = find_primes2(sublower, subupper);
                for (int k = 0; k < result.size; k++)
                {
                    // printf("%d ", result.primes[k]);
                    int bytes_written = write(fd, &result.primes[k], sizeof(int));
                    if (bytes_written == -1)
                    {
                        perror("write");
                        close(fd);
                        return 1;
                    }
                    // printf("Number of bytes written: %d", bytes_written);
                }
                printf("\n");
                // Close the named pipe
                close(fd);
            }
            exit(0);
        }
        else if (pid < 0)
        {
            /* Error */
            printf("Error");
            exit(1);
        }
        else
        {
            // parent process
            int fd_r = open(fifonames[i + j * n], O_RDONLY);
            int prime;
            int bytes_read = read(fd_r, &prime, sizeof(int));
            if (bytes_read == -1)
            {
                perror("read");
                close(fd_r);
                return 1;
            }
            while (bytes_read > 0)
            {
                num_primes++;
                printf("%d ", prime);
                bytes_read = read(fd_r, &prime, sizeof(int));
                if (bytes_read == -1)
                {
                    perror("read");
                    close(fd_r);
                    return 1;
                }
            }

            close(fd_r);
            wait(NULL);
        }
        flag = !flag;
    }
    printf("Number of primes: %d\n", num_primes);
    return 0;
}

// void cleanup_handler(int sig, char **fifo_names, int num_pipes)
// {
//     // Close and remove all open named pipes
//     for (int i = 0; i < num_pipes; i++)
//     {
//         if (access(fifo_names[i], F_OK))
//         {
//             if (close(fifo_names[i]) == -1)
//             {
//                 perror("Error closing pipe");
//             }
//             if (unlink(fifo_names[i]) == -1)
//             {
//                 perror("Error removing pipe");
//             }
//         }
//     }
//     // Exit the program
//     exit(0);
// }
int main(int argc, char *argv[])
{

    /* Check flags */
    int lower = 0;
    int upper = 0;
    bool equal = false;
    bool random = false;
    int n = 0;
    int num_primes = 0;

    // --- Testing generate_random_intervals
    // n = 5;
    // int* random_intervals = generate_random_intervals(1, 25, n);
    // for (int i = 0; i < 2 * n; i+= 2)
    // {
    //     printf("%d %d\n", random_intervals[i], random_intervals[i+1]);
    // }
    // ---

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
    int counter = 0;
    /* Create n*n worker named pipes*/
    char **fifo_names = malloc(sizeof(char *) * n * n);
    for (int k = 0; k < n * n; k++)
    {
        char *fifo_name = malloc(sizeof(char) * 10);
        sprintf(fifo_name, "fifo%d", k + 1);
        fifo_names[k] = fifo_name;
        printf("fifo_name: %s\n", fifo_name);
        if (mkfifo(fifo_name, 0666) == -1)
        {
            printf("Error creating named pipe");
            exit(1);
        }
    }

    /* Create n child processes */
    for (int j = 0; j < n; j++)
    {
        int pid = fork();
        if (pid == 0)
        {
            /* Child process */
            int upper_bound = 0;
            int lower_bound = 0;
            printf("Delegator process %d\n", j);
            if (random) {
                int *random_intervals = generate_random_intervals(lower, upper, n);
                upper_bound = random_intervals[2 * j + 1];
                lower_bound = random_intervals[2 * j];
            } else {
                /* Each delegator takes 1/n range from upper and lower bounds */
                upper_bound = upper - (upper - lower) / n * j;
                lower_bound = upper - (upper - lower) / n * (j + 1) + 1;
                if (j == n - 1)
                {
                    lower_bound = lower;
                }
            }
                printf("upper_bound: %d\n", upper_bound);
                printf("lower_bound: %d\n", lower_bound);
                delegator(j, n, upper_bound, lower_bound, fifo_names);
                exit(0);
        }
        else if (pid < 0)
        {
            /* Error */
            printf("Error");
            exit(1);
        }
        else
        {
            // parent process
            wait(NULL);
        }
    }

    /* Free all the named pipes */
    for (int t = 0; t < n * n; t++)
    {
        if (unlink(fifo_names[t]) == -1)
        {
            perror("unlink");
            return 1;
        }
        else
        {
            // printf("Unlinked %s\n", fifo_names[t]);
        }
        free(fifo_names[t]);
    }
    free(fifo_names);
    return 0;
}
