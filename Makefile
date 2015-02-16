CFLAGS=-Wall -pedantic -ggdb -std=c99

bin/main.o: main.c bin/short.o bin/hasht.o | bin
	gcc $(CFLAGS) -o bin/main.o bin/short.o bin/hasht.o main.c

bin/short.o: short.c | bin
	gcc -c $(CFLAGS) -o bin/short.o short.c

bin/hasht.o: hasht.h hasht.c | bin
	gcc -c $(CFLAGS) -o bin/hasht.o hasht.c

bin:
	mkdir -p bin

clean:
	rm bin/*
