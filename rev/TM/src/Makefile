$CC=gcc

tm: tm.o
	${CC} tm.o -o tm -s

tm.o: tm.c tm.h
	gcc -c tm.c

clean:
	rm *.o tm
