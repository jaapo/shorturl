CFLAGS=-Wall -pedantic -ggdb -std=c99

bin/main.o: main.c bin/short.o bin/hasht.o
	gcc $(CFLAGS) -o bin/main.o bin/short.o bin/hasht.o main.c

bin/short.o: short.c bin/hasht.o
	gcc -c $(CFLAGS) -o bin/short.o bin/hasht.o short.c

bin/hasht.o: hasht.h hasht.c
	gcc -c $(CFLAGS) -o bin/hasht.o hasht.c

clean:
	rm bin/*
