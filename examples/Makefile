SUBDIRS = $(shell ls -d */)

all:
	for dir in $(SUBDIRS) ; do \
		mingw32-make -C  $$dir ; \
		done
