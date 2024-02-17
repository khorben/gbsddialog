PREFIX	= /usr/local
OSNAME	= FreeBSD
DATADIR	= $(PREFIX)/share
INSTALL	= install
MKDIR	= mkdir -p -m 0755
RM	= rm -f
UNAME	= uname

all:

clean:

distclean:

install:
	[ $$($(UNAME) -s) != $(OSNAME) ] || \
		$(MKDIR) $(DESTDIR)$(DATADIR)/icons/hicolor/48x48/apps
	[ $$($(UNAME) -s) != $(OSNAME) ] || \
		$(INSTALL) -m 0644 bsdconfig-48.png \
			$(DESTDIR)$(DATADIR)/icons/hicolor/48x48/apps/bsdconfig.png
	[ $$($(UNAME) -s) != $(OSNAME) ] || \
		$(MKDIR) $(DESTDIR)$(DATADIR)/applications
	[ $$($(UNAME) -s) != $(OSNAME) ] || \
		$(INSTALL) -m 0644 bsdconfig.desktop \
			$(DESTDIR)$(DATADIR)/applications/bsdconfig.desktop

uninstall:
	$(RM) $(DESTDIR)$(DATADIR)/icons/hicolor/48x48/apps/bsdconfig.png
	$(RM) $(DESTDIR)$(DATADIR)/applications/bsdconfig.desktop

.PHONY: all clean distclean install uninstall
