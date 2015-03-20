all: delwords

delwords:
	gcc -std=c99 -I../lib -L../lib -Wl,-rpath=../lib delwords.c -lhelpers -o delwords

clean:
	rm -f ./delwords
