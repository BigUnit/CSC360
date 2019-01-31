CC=gcc

kapish : kapish.o 
	$(CC) -Wall -Werror kapish.c -o  kapish

clean: 
	-rm kapish kapish.o
