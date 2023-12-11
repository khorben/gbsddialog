PACKAGE	= gbsddialog
VERSION	= 0.0.1
SUBDIRS	= src tools
LN	= ln -f
RM	= rm -f
TAR	= tar

all:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE)) || break; done

clean:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) clean) || break; done

dist:
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- "$$PWD" $(PACKAGE)-$(VERSION)
	$(TAR) -czf $(PACKAGE)-$(VERSION).tar.gz \
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
		$(PACKAGE)-$(VERSION)/tools/Xdialog.c
	$(RM) -- $(PACKAGE)-$(VERSION)

distclean:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) distclean) || break; done

install:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) install) || break; done

uninstall:
	@for subdir in $(SUBDIRS); do (cd $$subdir && $(MAKE) uninstall) || break; done

.PHONY: all clean dist distclean install uninstall
