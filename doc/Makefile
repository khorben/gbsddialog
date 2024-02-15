PROJECT	= gbsddialog
VERSION	= 0.0.1
TARGETS	= Xdialog.1
SAMPLES	= Xdialog.wrapper.c Xmessage Xmore allrpms bluebox.rc buildlist \
	  calendar chain checklist dselect editbox fixed-font.rc format1440 \
	  fselect gauge infobox infobox2 inputbox install-wrapper kernel \
	  logbox logbox2 login menubox msgbox no-entry.xpm radiolist rangebox \
	  set-time tailbox textbox timebox treeview warning.xpm xlock-wrapper \
	  yesno
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
DATADIR	= $(PREFIX)/share
MANDIR	= $(DATADIR)/man
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f
SED	= sed

all: $(TARGETS)

Xdialog.1: Xdialog.1.in
	$(SED) -e s,%%project%%,$(PROJECT), \
		-e s,%%version%%,$(VERSION), \
		-e s,%%prefix%%,$(PREFIX), \
		-e s,%%bindir%%,$(BINDIR), \
		-e s,%%datadir%%,$(DATADIR), \
		-e s,%%mandir%%,$(MANDIR), \
		Xdialog.1.in > Xdialog.1 || $(RM) Xdialog.1

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(DATADIR)/doc/$(PROJECT)/samples
	for sample in $(SAMPLES); do \
		mode=0644; [ -x samples/$$sample ] && mode=0755; \
		$(INSTALL) -m $$mode samples/$$sample \
			$(DESTDIR)$(DATADIR)/doc/$(PROJECT)/samples/$$sample \
				|| exit $$?; \
	done
	$(MKDIR) $(DESTDIR)$(MANDIR)/man1
	$(INSTALL) -m 0644 gbsddialog.1 $(DESTDIR)$(MANDIR)/man1/gbsddialog.1
	$(INSTALL) -m 0644 Xdialog.1 $(DESTDIR)$(MANDIR)/man1/Xdialog.1

uninstall:
	for sample in $(SAMPLES); do \
		$(RM) $(DESTDIR)$(DATADIR)/doc/$(PROJECT)/samples/$$sample \
				|| exit $$?; \
	done
	$(RM) $(DESTDIR)$(MANDIR)/man1/gbsddialog.1
	$(RM) $(DESTDIR)$(MANDIR)/man1/Xdialog.1

.PHONY: all clean distclean install uninstall
