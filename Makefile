SUBDIRS	= src tools

all:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE)) || break; done

clean:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) clean) || break; done

distclean:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) distclean) || break; done

install:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) install) || break; done

uninstall:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) uninstall) || break; done

.PHONY: all clean distclean install uninstall
