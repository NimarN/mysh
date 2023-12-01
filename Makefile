CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -fsanitize=address,undefined




arraylist.o: arraylist.c arraylist.h
	$(CC) $(CFLAGS) -c arraylist.c

mysh: mysh.o wildcard.o arraylist.o
	$(CC) $(CFLAGS) mysh.o wildcard.o arraylist.o -o mysh

mysh.o: mysh.c wildcard.c  arraylist.h
	$(CC) $(CFLAGS) mysh.c wildcard.c -c mysh.c

clean:
	rm -f mysh *.o