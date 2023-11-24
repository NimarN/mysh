CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -fsanitize=address,undefined


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

testsh: testsh.o wildcard.o
	$(CC) $(CFLAGS) testsh.o wildcard.o -o testsh

clean:
	rm -f *.o