PACKAGE	= gbsddialog
VERSION	= 0.0.1
SUBDIRS	= src tools
LN	= ln -f
RM	= rm -f
TAR	= tar

all:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE)) || exit $$?; done

clean:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) clean) || exit $$?; done

dist:
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- "$$PWD" $(PACKAGE)-$(VERSION)
	$(TAR) -czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/README.md \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/builders.c \
		$(PACKAGE)-$(VERSION)/src/callbacks.c \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/gbsddialog.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/builders.h \
		$(PACKAGE)-$(VERSION)/src/callbacks.h \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/gbsddialog.h \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/builders.c \
		$(PACKAGE)-$(VERSION)/tools/Xdialog.c
	$(RM) -- $(PACKAGE)-$(VERSION)

distclean:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) distclean) || exit $$?; done

install:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) install) || exit $$?; done

uninstall:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) uninstall) || exit $$?; done

.PHONY: all clean dist distclean install uninstall
