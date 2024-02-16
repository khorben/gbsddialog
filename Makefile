PACKAGE	= gbsddialog
VERSION	= 0.8.0
SUBDIRS	= data doc src tests tools
PREFIX	= /usr/local
DATADIR	= $(PREFIX)/share
INSTALL	= install
LN	= ln -f
MKDIR	= mkdir -p
RM	= rm -f
TAR	= tar

all:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE)) || exit $$?; \
	done

tests: all
	cd tests && $(MAKE) tests

clean:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) clean) || exit $$?; \
	done

dist:
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- "$$PWD" $(PACKAGE)-$(VERSION)
	$(TAR) -czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/README.md \
		$(PACKAGE)-$(VERSION)/data/Makefile \
		$(PACKAGE)-$(VERSION)/data/bsdconfig-48.png \
		$(PACKAGE)-$(VERSION)/data/bsdconfig.desktop \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/gbsddialog.1 \
		$(PACKAGE)-$(VERSION)/doc/samples/Xdialog.wrapper.c \
		$(PACKAGE)-$(VERSION)/doc/samples/Xmessage \
		$(PACKAGE)-$(VERSION)/doc/samples/Xmore \
		$(PACKAGE)-$(VERSION)/doc/samples/allrpms \
		$(PACKAGE)-$(VERSION)/doc/samples/bluebox.rc \
		$(PACKAGE)-$(VERSION)/doc/samples/buildlist \
		$(PACKAGE)-$(VERSION)/doc/samples/calendar \
		$(PACKAGE)-$(VERSION)/doc/samples/chain \
		$(PACKAGE)-$(VERSION)/doc/samples/checklist \
		$(PACKAGE)-$(VERSION)/doc/samples/dselect \
		$(PACKAGE)-$(VERSION)/doc/samples/editbox \
		$(PACKAGE)-$(VERSION)/doc/samples/fixed-font.rc \
		$(PACKAGE)-$(VERSION)/doc/samples/format1440 \
		$(PACKAGE)-$(VERSION)/doc/samples/fselect \
		$(PACKAGE)-$(VERSION)/doc/samples/gauge \
		$(PACKAGE)-$(VERSION)/doc/samples/infobox \
		$(PACKAGE)-$(VERSION)/doc/samples/infobox2 \
		$(PACKAGE)-$(VERSION)/doc/samples/inputbox \
		$(PACKAGE)-$(VERSION)/doc/samples/install-wrapper \
		$(PACKAGE)-$(VERSION)/doc/samples/kernel \
		$(PACKAGE)-$(VERSION)/doc/samples/logbox \
		$(PACKAGE)-$(VERSION)/doc/samples/logbox2 \
		$(PACKAGE)-$(VERSION)/doc/samples/login \
		$(PACKAGE)-$(VERSION)/doc/samples/menubox \
		$(PACKAGE)-$(VERSION)/doc/samples/msgbox \
		$(PACKAGE)-$(VERSION)/doc/samples/no-entry.xpm \
		$(PACKAGE)-$(VERSION)/doc/samples/radiolist \
		$(PACKAGE)-$(VERSION)/doc/samples/rangebox \
		$(PACKAGE)-$(VERSION)/doc/samples/set-time \
		$(PACKAGE)-$(VERSION)/doc/samples/tailbox \
		$(PACKAGE)-$(VERSION)/doc/samples/textbox \
		$(PACKAGE)-$(VERSION)/doc/samples/timebox \
		$(PACKAGE)-$(VERSION)/doc/samples/treeview \
		$(PACKAGE)-$(VERSION)/doc/samples/warning.xpm \
		$(PACKAGE)-$(VERSION)/doc/samples/xlock-wrapper \
		$(PACKAGE)-$(VERSION)/doc/samples/yesno \
		$(PACKAGE)-$(VERSION)/doc/Xdialog.1.in \
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
		$(PACKAGE)-$(VERSION)/tests/Makefile \
		$(PACKAGE)-$(VERSION)/tests/gbsddialog.c \
		$(PACKAGE)-$(VERSION)/tests/Xdialog.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/builders.c \
		$(PACKAGE)-$(VERSION)/tools/Xdialog.c
	$(RM) -- $(PACKAGE)-$(VERSION)

distcheck: dist
	$(TAR) -xzf $(PACKAGE)-$(VERSION).tar.gz
	cd $(PACKAGE)-$(VERSION) && $(MAKE) dist all

distclean:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) distclean) || exit $$?; \
	done

install:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) install) || exit $$?; \
	done
	$(MKDIR) $(DESTDIR)$(DATADIR)/doc/$(PACKAGE)
	$(INSTALL) -m 0644 COPYING $(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/COPYING
	$(INSTALL) -m 0644 README.md \
		$(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/README.md

uninstall:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) uninstall) || exit $$?; \
	done
	$(RM) $(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/COPYING
	$(RM) $(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/README.md

.PHONY: all tests clean dist distcheck distclean install uninstall
