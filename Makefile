CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -fsanitize=address,undefined


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

testsh: testsh.o wildcard.o arraylist.o
	$(CC) $(CFLAGS) testsh.o wildcard.o arraylist.o -o testsh

clean:
	rm -f *.o