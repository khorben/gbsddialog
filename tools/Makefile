TARGETS	= Xdialog
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
CPPFLAGS= -D_FORTIFY_SOURCE=2 -DWITH_XDIALOG
CFLAGS	= -W -Wall -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS	= `pkg-config --libs gtk+-3.0`
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f
OBJS	= Xdialog.o

all: $(TARGETS)

Xdialog: $(OBJS)
	$(CC) $(LDFLAGS) -o Xdialog $(OBJS)

Xdialog.o: Xdialog.c ../src/gbsddialog
	$(CC) $(CPPFLAGS) $(CFLAGS) -o Xdialog.o -c Xdialog.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(BINDIR)
	$(INSTALL) -m 0755 Xdialog $(BINDIR)/Xdialog

uninstall:
	$(RM) $(BINDIR)/Xdialog

.PHONY: all clean distclean install uninstall
