SUBDIRS = $(shell ls -d */)

.PHONY: $(SUBDIRS) clean

$(SUBDIRS):
	$(MAKE) -C $@

.DEFAULT_GOAL := all
all: $(SUBDIRS)

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

print-%  : ; @echo $* = $($*)


