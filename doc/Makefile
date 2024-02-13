PROJECT	= gbsddialog
VERSION	= 0.0.1
TARGETS	= Xdialog.1
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

install:
	$(MKDIR) $(MANDIR)/man1
	$(INSTALL) -m 0644 gbsddialog.1 $(MANDIR)/man1/gbsddialog.1
	$(INSTALL) -m 0644 Xdialog.1 $(MANDIR)/man1/Xdialog.1

uninstall:
	$(RM) $(MANDIR)/man1/gbsddialog.1
	$(RM) $(MANDIR)/man1/Xdialog.1

.PHONY: all clean distclean install uninstall
