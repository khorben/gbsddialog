PREFIX	= /usr/local
DATADIR	= $(PREFIX)/share
MANDIR	= $(DATADIR)/man
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f

all:

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install:
	$(MKDIR) $(MANDIR)/man1
	$(INSTALL) -m 0644 gbsddialog.1 $(MANDIR)/man1/gbsddialog.1
	$(INSTALL) -m 0644 Xdialog.1 $(MANDIR)/man1/Xdialog.1

uninstall:
	$(RM) $(MANDIR)/man1/gbsddialog.1
	$(RM) $(MANDIR)/man1/Xdialog.1

.PHONY: all clean distclean install uninstall
