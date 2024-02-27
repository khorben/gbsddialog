PACKAGE	= gbsddialog
VERSION	= 0.9.0
TARGETS	= $(OBJDIR)Xdialog.1
OBJS	=
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
MKDIR	= mkdir -p -m 0755
RM	= rm -f
SED	= sed

all: $(TARGETS)

$(OBJDIR)Xdialog.1: Xdialog.1.in Makefile
	$(SED) -e s,%%project%%,$(PACKAGE), \
		-e s,%%version%%,$(VERSION), \
		-e s,%%prefix%%,$(PREFIX), \
		-e s,%%bindir%%,$(BINDIR), \
		-e s,%%datadir%%,$(DATADIR), \
		-e s,%%mandir%%,$(MANDIR), \
		Xdialog.1.in > $(OBJDIR)Xdialog.1 || $(RM) $(OBJDIR)Xdialog.1

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: all
	$(MKDIR) $(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/samples
	for sample in $(SAMPLES); do \
		mode=0644; [ -x samples/$$sample ] && mode=0755; \
		$(INSTALL) -m $$mode samples/$$sample \
			$(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/samples/$$sample \
				|| exit $$?; \
	done
	$(MKDIR) $(DESTDIR)$(MANDIR)/man1
	$(INSTALL) -m 0644 gbsddialog.1 $(DESTDIR)$(MANDIR)/man1/gbsddialog.1
	$(INSTALL) -m 0644 $(OBJDIR)Xdialog.1 $(DESTDIR)$(MANDIR)/man1/Xdialog.1

uninstall:
	for sample in $(SAMPLES); do \
		$(RM) $(DESTDIR)$(DATADIR)/doc/$(PACKAGE)/samples/$$sample \
				|| exit $$?; \
	done
	$(RM) $(DESTDIR)$(MANDIR)/man1/gbsddialog.1
	$(RM) $(DESTDIR)$(MANDIR)/man1/Xdialog.1

.PHONY: all clean distclean install uninstall
