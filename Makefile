all:
	make -C lib/
	make -C bufcat/
	make -C cat/
	make -C revwords/
	make -C filter/

clean:
	make clean -C lib/
	make clean -C bufcat/
	make clean -C cat/
	make clean -C revwords/
	make clean -C filter/
