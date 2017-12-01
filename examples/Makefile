SUBDIRS = $(shell ls -d */)

.PHONY: $(SUBDIRS)


$(SUBDIRS):
	$(MAKE) -C $@

.DEFAULT_GOAL := all
all: $(SUBDIRS)


print-%  : ; @echo $* = $($*)
