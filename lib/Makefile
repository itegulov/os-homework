all:
	gcc -std=c99 -c -Wall -fpic helpers.c -o helpers.o
	gcc -std=c99 -shared -o libhelpers.so helpers.o

clean:
	rm -f *.o *.so
