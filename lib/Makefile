all:
	gcc -c -Wall -fpic helpers.c -o helpers.o
	gcc -shared -o libhelpers.so helpers.o

clean:
	rm -f *.o *.so
