all: revwords
revwords:
	gcc -std=c99 -I../lib -L../lib -Wl,-rpath=../lib revwords.c -lhelpers -o revwords

clean:
	rm -f ./revwords
