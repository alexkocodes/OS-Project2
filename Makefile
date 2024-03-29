OBJS 	= primes.o
SOURCE	= primes.c
OUT  	= primes
CC	= gcc
FLAGS   = -g -c -pedantic -ansi  -Wall -std=c99
LIBS = -lm
# -g option enables debugging mode 
# -c flag generates object code for separate files

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $@ $(LIBS)

# create/compile the individual files >>separately<< 
primes.o: primes.c
	$(CC) $(FLAGS) primes.c $(LIBS)

# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)
