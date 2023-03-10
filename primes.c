#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Create functions to find primes for a given subrange
// --- Professor Provided functions to check if number is prime ---
int prime1(int n) {
  int i;
  if (n == 1)
    return 0;
  for (i = 2; i < n; i++)
    if (n % i == 0)
      return 0;
  return 1;
}

int prime2(int n) {
  int i = 0, limitup = 0;
  limitup = (int)(sqrt((float)n));
  if (n == 1)
    return 0;
  for (i = 2; i <= limitup; i++)
    if (n % i == 0)
      return 0;
  return 1;
}
// --- Professor Provided functions to check if number is prime ---

// --- Finding primes for a given subrange ---

/* Create a struct to hold the primes and the size of the array */
typedef struct {
  int *primes; // array of primes in a given subrange
  int size;    // size of the array
} primes_in_subrange;

/* Create functions to find primes for a given subrange */
primes_in_subrange find_primes1(int lower, int upper) {
  int *primes = malloc(sizeof(int) * (upper - lower));
  int size = 0;
  for (int i = lower; i < upper; i++) {
    if (prime1(i)) {
      primes[size] = i;
      size++;
    }
  }
  primes_in_subrange result = {primes, size};
  return result;
}

primes_in_subrange find_primes2(int lower, int upper) {
  int *primes = malloc(sizeof(int) * (upper - lower));
  int size = 0;
  for (int i = lower; i < upper; i++) {
    if (prime2(i)) {
      primes[size] = i;
      size++;
    }
  }
  primes_in_subrange result = {primes, size};
  return result;
}
// --- Finding primes for a given subrange ---

/*Create a function that creates n child processes */
int *delegator(int n, int upper, int lower) {
  for (int i = 0; i < n; i++) {
    int pid = fork();
    if (pid == 0) {
      /* Child process */
      printf("Child process %d", i);
      exit(0);
    } else if (pid < 0) {
      /* Error */
      printf("Error");
      exit(1);
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {

  primes_in_subrange result = find_primes1(1, 30);
  for (int i = 0; i < result.size; i++) {
    printf("%d ", result.primes[i]);
  }
  printf("\n");

  primes_in_subrange result2 = find_primes2(1, 30);
  for (int i = 0; i < result2.size; i++) {
    printf("%d ", result2.primes[i]);
  }
  printf("\n");

  /* Check flags */
  int lower = 0;
  int upper = 0;
  bool equal = false;
  bool random = false;
  int n = 0;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) {
      lower = atoi(argv[i + 1]);
      printf("lower: %d\n", lower);
    }
    if (strcmp(argv[i], "-u") == 0) {
      upper = atoi(argv[i + 1]);
      printf("upper: %d\n", upper);
    }
    if (strcmp(argv[i], "-e") == 0) {
      equal = true;
      printf("equal: %d\n", equal);
    }
    if (strcmp(argv[i], "-r") == 0) {
      random = true;
      printf("random: %d\n", random);
    }
    if (strcmp(argv[i], "-n") == 0) {
      n = atoi(argv[i + 1]);
      printf("n: %d\n", n);
    }
  }
  /* Create n child processes */
  for (int j = 0; j < n; j++) {
    int pid = fork();
    if (pid == 0) {
      /* Child process */
      printf("Delegator process %d\n", j);
      /* Each delegator takes 1/n range from upper and lower bounds */
      int upper_bound = upper - (upper - lower) / n * j;
      int lower_bound = upper - (upper - lower) / n * (j + 1);
      printf("upper_bound: %d\n", upper_bound);
      printf("lower_bound: %d\n", lower_bound);
      // delegator(n, upper_bound, lower_bound);
      exit(0);
    } else if (pid < 0) {
      /* Error */
      printf("Error");
      exit(1);
    }
  }
  return 0;
};