SUBDIRS = lib cat filter revwords bufcat simplesh filesender bipiper
.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	make -C $@

clean:
	for sd in $(SUBDIRS); do \
		make -C $$sd clean; \
	done
