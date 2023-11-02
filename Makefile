SUBDIRS	= src

all:
	@for subdir in $(SUBDIRS); do cd $$subdir && $(MAKE); done

clean:
	@for subdir in $(SUBDIRS); do cd $$subdir && $(MAKE) clean; done

distclean:
	@for subdir in $(SUBDIRS); do cd $$subdir && $(MAKE) distclean; done

install:
	@for subdir in $(SUBDIRS); do cd $$subdir && $(MAKE) install; done

uninstall:
	@for subdir in $(SUBDIRS); do cd $$subdir && $(MAKE) uninstall; done

.PHONY: all clean distclean install uninstall
