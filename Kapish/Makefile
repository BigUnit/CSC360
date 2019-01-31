CC=gcc

kapish : kapish.o 
	$(CC)  kapish.o -o  kapish

kapish.o : kapish.c
	$(CC) -c -std=c11 -Wall -Werror kapish.c -o  kapish.o

clean: 
	-rm kapish kapish.o
