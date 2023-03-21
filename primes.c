/* Group Parners: Alex Ko (fyk211) and Ritin Malhotra (rm5486)*/

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
#include <signal.h>
#include <sys/select.h>

// comparison function for qsort method
int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}
// q sort function
void qsort_primes(int *primes, int size)
{
    qsort(primes, size, sizeof(int), compare);
}

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
int *generate_random_intervals(int lower, int upper, int n)
{
    srand(time(0));

    // Create 2 * n "points" randomly in the range [lower, upper]
    // Ensuring that the first point == lower and the (n + 1)th point == upper
    int *points = malloc(sizeof(int) * 2 * n);

    points[0] = lower;
    points[(2 * n) - 1] = upper;

    // If upper - lower + 1 < 2 * n, then we can't create n intervals
    // In that case, create as many intervals as possible and fill the rest with -1
    if (upper - lower + 1 < 2 * n)
    {
        for (int i = 1; i < (2 * n) - 1; i += 2)
        {
            points[i] = lower + i / 2;
            points[i + 1] = lower + i / 2 + 1;
        }
        return points;
    }

    // If lower == upper, then we can just return the points
    if (lower == upper)
    {
        return points;
    }

    for (int i = 1; i < (2 * n) - 1; i += 2)
    {
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

/*function that creates n child processes */
int delegator(int j, int n, int upper, int lower, char **fifonames, char **delegator_pipes, bool random)
{
    int num_primes = 0;
    char *delegator_pipe = delegator_pipes[j]; // get the pipe name for the current delegator, used for passing the primes to the parent process
    bool flag = true;                          // if flag is true, then we assign function 1 to the first child process and flip
    if (n % 2 != 0 && j % 2 == 0)
    {
        flag = false; // if n is odd and j is even, then we assign function 2 to the first child process and flip
    }
    for (int i = 0; i < n; i++) // create n child worker processes here
    {
        int fd_r = open(fifonames[i + j * n], O_RDONLY | O_NONBLOCK); // open the named pipe for read for each worker
        int pid = fork();
        if (pid == 0)
        {
            /* Child process */
            /*printf("Child process %d\n", i);*/
            /*printf("Named pipe: %s\n", fifonames[i + j * n]);*/

            // Open signal pipe for write for passing the signals
            int fd_s = open("signal_pipe", O_WRONLY | O_NONBLOCK);
            if (fd_s == -1)
            {
                perror("Error opening signal pipe\n");
                exit(1);
            }

            /* Open time pipe for write */
            int fd_t = open("time_pipe", O_WRONLY | O_NONBLOCK);
            if (fd_t == -1)
            {
                perror("Error opening time pipe\n");
                exit(1);
            }
            /* Open named pipe */
            int fd = open(fifonames[i + j * n], O_WRONLY | O_NONBLOCK);
            if (fd == -1)
            {
                perror("Error opening named pipe\n");
                exit(1);
            }
            int sublower = 0;
            int subupper = 0;
            if (!random) // if random is false, then we split the range into n subranges
            {
                sublower = lower + (upper - lower) / n * i;
                subupper = lower + (upper - lower) / n * (i + 1) - 1;
                if (i == n - 1)
                {
                    subupper = upper;
                }
            }
            else // if random is true, then we split the range into n subranges randomly
            {
                int *points = generate_random_intervals(lower, upper, n);
                sublower = points[2 * i];
                subupper = points[2 * i + 1];
            }

            // once the subrange is determined, we find the primes in that subrange
            if (flag) // flag is used to delegate the function 1, 2 in a circular fashion
            {
                // split the range into n subranges and find the primes in each subrange
                // printf("Finding primes in range %d to %d using function 1\n", sublower, subupper);
                time_t start, end;
                start = clock(); // start the timer
                primes_in_subrange result = find_primes1(sublower, subupper);
                for (int k = 0; k < result.size; k++)
                {
                    // printf("%d ", result.primes[k]);
                    int bytes_written = write(fd, &result.primes[k], sizeof(int)); // write the primes to the named pipe
                    if (bytes_written == -1)
                    {
                        perror("write");
                        close(fd);
                        return 1;
                    }
                }
                end = clock(); // end the timer
                double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
                /*printf("Time taken: %f\n", time_taken);*/
                /* Write to time pipe */
                int bytes_written = write(fd_t, &time_taken, sizeof(double)); // write the time taken to the time pipe
                if (bytes_written == -1)
                {
                    perror("write");
                    close(fd_t);
                    return 1;
                }
                // Close the named pipe
                close(fd);
            }
            else
            {
                // split the range into n subranges and find the primes in each subrange
                // printf("Finding primes in range %d to %d using function 2\n", sublower, subupper);
                time_t start, end;
                start = clock(); // start the timer
                primes_in_subrange result = find_primes2(sublower, subupper);
                for (int k = 0; k < result.size; k++)
                {
                    // printf("%d ", result.primes[k]);
                    int bytes_written = write(fd, &result.primes[k], sizeof(int)); // write the primes to the named pipe
                    if (bytes_written == -1)
                    {
                        perror("write");
                        close(fd);
                        return 1;
                    }
                    // printf("Number of bytes written: %d", bytes_written);
                }
                end = clock(); // end the timer
                double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
                /*printf("Time taken: %f\n", time_taken);*/
                /* Write to time pipe */
                int bytes_written = write(fd_t, &time_taken, sizeof(double)); // write the time taken to the time pipe
                if (bytes_written == -1)
                {
                    perror("write");
                    close(fd_t);
                    return 1;
                }
                // Close the named pipe
                close(fd);
            }
            // Write signal to signal pipe
            // First, we need to determine the signal to send
            int signal;
            if (j % 2 == 0) // "even delegator" -> send SIGUSR1
            {
                signal = SIGUSR1;
            }
            else // "odd delegator" -> send SIGUSR2
            {
                signal = SIGUSR2;
            }
            // Secondly, write the signal to the signal pipe
            int bytes_written = write(fd_s, &signal, sizeof(int));
            if (bytes_written == -1)
            {
                perror("write");
                close(fd_s);
                return 1;
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
            int fd_r = open(fifonames[i + j * n], O_RDONLY | O_NONBLOCK);
            int fd_w = open(delegator_pipe, O_WRONLY | O_NONBLOCK);
            int prime;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd_r, &fds);
            int num_ready = select(fd_r + 1, &fds, NULL, NULL, NULL); // if there is data to be read
            if (num_ready == -1)
            {
                perror("select");
                return 1;
            }
            if (num_ready > 0)
            {
                int bytes_read = read(fd_r, &prime, sizeof(int)); // read the prime from the named pipe
                if (bytes_read == -1)
                {
                    perror("read");
                    close(fd_r);
                    return 1;
                }
                while (bytes_read > 0)
                {
                    num_primes++;
                    /* write to the delegator pipe*/
                    int bytes_written = write(fd_w, &prime, sizeof(int));
                    if (bytes_written == -1)
                    {
                        perror("write");
                        close(fd_w);
                        return 1;
                    }
                    else
                    {
                        // printf("Written back to the delegator pipe with %d", prime);
                    }
                    num_ready = select(fd_r + 1, &fds, NULL, NULL, NULL);
                    if (num_ready > 0)
                    {
                        bytes_read = read(fd_r, &prime, sizeof(int));
                        if (bytes_read == -1)
                        {
                            perror("read");
                            close(fd_r);
                            return 1;
                        }
                    }
                }
            }
            wait(NULL);
        }
        flag = !flag; // toggle the flag so that the next worker will use the other function
    }
    // printf("Number of primes in delegator %d: %d\n", j, num_primes);
    return 0;
}

void cleanup_handler(int sig) // function used to delete pipes from the file system, if the user terminates the program with control + c
{
    int status = system("Rm fifo*; rm de*; rm time*; rm signal_pipe*");
    if (status == -1)
    {
        perror("system");
        exit(1);
    }
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, cleanup_handler); // register the cleanup handler

    /* Check flags */
    int lower = 0;
    int upper = 0;
    bool equal = false;
    bool random = false;
    int n = 0;
    int num_primes = 0;

    int i = 0;
    for (i = 0; i < argc; i++)
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
            // if n is too big, then terminate the program
            if (n > 10)
            {
                printf("n is too big. Please enter a value less than 10.\n");
                exit(1);
            }
            /*printf("n: %d\n", n);*/
        }
    }
    int counter = 0;
    int *result = malloc(sizeof(int) * upper);
    /* Create n*n worker named pipes*/
    char **fifo_names = malloc(sizeof(char *) * n * n);
    int k;
    for (k = 0; k < n * n; k++)
    {
        char *fifo_name = malloc(sizeof(char) * 10);
        sprintf(fifo_name, "fifo%d", k + 1);
        fifo_names[k] = fifo_name;
        // printf("fifo_name: %s\n", fifo_name);
        if (mkfifo(fifo_name, 0666) == -1)
        {
            printf("Error creating named pipe");
            exit(1);
        }
    }
    /* Create n named pipes for communications between root and delegators */
    char **delegator_pipes = malloc(sizeof(char *) * n);
    for (k = 0; k < n; k++)
    {
        char *delegator_pipe = malloc(sizeof(char) * 10);
        sprintf(delegator_pipe, "delegator_pipe%d", k + 1);
        delegator_pipes[k] = delegator_pipe;
        /*printf("delegator_pipe: %s\n", delegator_pipe);*/
        if (mkfifo(delegator_pipe, 0666) == -1)
        {
            printf("Error creating named pipe");
            exit(1);
        }
    }
    /* Create a named pipe for passing time taken by each worker node*/
    if (mkfifo("time_pipe", 0666) == -1)
    {
        printf("Error creating named pipe");
        exit(1);
    }
    int time_pipe_fd = open("time_pipe", O_RDONLY | O_NONBLOCK);

    int sigusr1_count = 0;
    int sigusr2_count = 0;

    // Create a neamed pipe for passing SIGUSR1 or SIGUSR2 from worker to root
    if (mkfifo("signal_pipe", 0666) == -1)
    {
        printf("Error creating named pipe");
        exit(1);
    }
    int signal_pipe_fd = open("signal_pipe", O_RDONLY | O_NONBLOCK);

    /* Create n child processes */
    for (int j = 0; j < n; j++)
    {
        int fd_r = open(delegator_pipes[j], O_RDONLY | O_NONBLOCK);
        int pid = fork();
        if (pid == 0)
        {
            /* Child process */
            int upper_bound = 0;
            int lower_bound = 0;
            /*printf("Delegator process %d\n", j);*/
            if (random)
            {
                int *random_intervals = generate_random_intervals(lower, upper, n);
                upper_bound = random_intervals[2 * j + 1];
                lower_bound = random_intervals[2 * j];
            }
            else
            {
                /* Each delegator takes 1/n range from upper and lower bounds */
                upper_bound = upper - (upper - lower) / n * j;
                lower_bound = upper - (upper - lower) / n * (j + 1) + 1;
                if (j == n - 1)
                {
                    lower_bound = lower;
                }
            }
            /*
            printf("upper_bound: %d\n", upper_bound);
            printf("lower_bound: %d\n", lower_bound);
            */
            delegator(j, n, upper_bound, lower_bound, fifo_names, delegator_pipes, random);
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
            int fd_r = open(delegator_pipes[j], O_RDONLY | O_NONBLOCK);
            int prime;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd_r, &fds);
            int num_ready = select(fd_r + 1, &fds, NULL, NULL, NULL);
            if (num_ready == -1)
            {
                perror("select");
                return 1;
            }
            if (num_ready > 0)
            {
                int bytes_read = read(fd_r, &prime, sizeof(int)); // read the first prime
                if (bytes_read == -1)
                {
                    perror("read");
                    close(fd_r);
                    return 1;
                }
                while (bytes_read > 0) // read all primes
                {
                    if (prime <= upper)
                    {
                        num_primes++;
                        /* Add prime to the result array */
                        result[counter] = prime;
                        counter++;
                        num_ready = select(fd_r + 1, &fds, NULL, NULL, NULL);
                        if (num_ready > 0)
                        {
                            bytes_read = read(fd_r, &prime, sizeof(int)); // read the next prime
                            if (bytes_read == -1)
                            {
                                perror("read");
                                close(fd_r);
                                return 1;
                            }
                        }
                    }
                }
            }
            wait(NULL);
        }
    }

    /* q sort the result array*/
    qsort(result, num_primes, sizeof(int), compare);
    /* Print the result */
    printf("Number of primes in total: %d\n", num_primes);
    printf("Primes: ");
    for (i = 0; i < num_primes; i++)
    {
        printf("%d ", result[i]);
    }
    printf("\n");
    free(result);

    /* Store the max, min, and average of time*/
    double max = 0;
    double min = 100000000;
    double sum = 0;

    int time_counter = 0;
    /* Open the read for time_pipe */
    if (time_pipe_fd == -1)
    {
        perror("open");
        return 1;
    }
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(time_pipe_fd, &fds);
    int num_ready = select(time_pipe_fd + 1, &fds, NULL, NULL, NULL);
    if (num_ready == -1)
    {
        perror("select");
        return 1;
    }
    if (num_ready > 0)
    {
        double time;
        int bytes_read = read(time_pipe_fd, &time, sizeof(double));

        if (bytes_read == -1)
        {
            perror("read");
            close(time_pipe_fd);
            return 1;
        }
        while (bytes_read > 0)
        {
            if (time > max) // update max
            {
                max = time;
            }
            if (time < min) // update min
            {
                min = time;
            }
            sum += time; // update sum
            time_counter++;
            num_ready = select(time_pipe_fd + 1, &fds, NULL, NULL, NULL);
            if (num_ready > 0)
            {
                bytes_read = read(time_pipe_fd, &time, sizeof(double));
                if (bytes_read == -1)
                {
                    perror("read");
                    close(time_pipe_fd);
                    return 1;
                }
            }
            // printf("Counter: %d, Time: %f\n", time_counter, time);
        }
    }

    printf("Max time: %f\n", max);
    printf("Min time: %f\n", min);
    printf("Average time: %f\n", sum / time_counter);

    // Open the read for signal_pipe
    if (signal_pipe_fd == -1)
    {
        perror("open");
        return 1;
    }
    fd_set fds2;
    FD_ZERO(&fds2);
    FD_SET(signal_pipe_fd, &fds2);
    num_ready = select(signal_pipe_fd + 1, &fds2, NULL, NULL, NULL);
    if (num_ready == -1)
    {
        perror("select");
        return 1;
    }
    if (num_ready > 0)
    {
        // printf("Signal pipe is ready\n");
        int signal;
        int bytes_read = read(signal_pipe_fd, &signal, sizeof(int));
        if (bytes_read == -1)
        {
            perror("read");
            close(signal_pipe_fd);
            return 1;
        }

        while (bytes_read > 0)
        {
            if (signal == SIGUSR1) // update sigusr1_count
            {
                sigusr1_count++;
            }
            else if (signal == SIGUSR2) // update sigusr2_count
            {
                sigusr2_count++;
            }

            num_ready = select(signal_pipe_fd + 1, &fds2, NULL, NULL, NULL);
            if (num_ready > 0)
            {
                bytes_read = read(signal_pipe_fd, &signal, sizeof(int));
                if (bytes_read == -1)
                {
                    perror("read");
                    close(signal_pipe_fd);
                    return 1;
                }
            }
        }
    }

    printf("SIGUSR1: %d\n", sigusr1_count);
    printf("SIGUSR2: %d\n", sigusr2_count);

    int t, d;
    /* Close all the delegator pipes*/
    for (d = 0; d < n; d++)
    {
        close(delegator_pipes[d]);
    }
    /* Close all the worker pipes*/
    for (t = 0; t < n * n; t++)
    {
        close(fifo_names[t]);
    }
    /* Free all the named pipes */
    for (t = 0; t < n * n; t++)
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

    /* Free all the delegator pipes*/
    for (d = 0; d < n; d++)
    {
        if (unlink(delegator_pipes[d]) == -1)
        {
            perror("unlink");
            return 1;
        }
        else
        {
            // printf("Unlinked %s\n", fifo_names[t]);
        }
        free(delegator_pipes[d]);
    }
    free(delegator_pipes);

    /* Free the time pipe */
    if (unlink("time_pipe") == -1)
    {
        perror("unlink");
        return 1;
    }
    else
    {
        // printf("Unlinked %s\n", fifo_names[t]);
    }

    /* Free the signal pipe */
    if (unlink("signal_pipe") == -1)
    {
        perror("unlink");
        return 1;
    }
    else
    {
        // printf("Unlinked %s\n", fifo_names[t]);
    }

    return 0;
}
