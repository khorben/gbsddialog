TARGETS	= Xdialog
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
CPPFLAGS= -D_FORTIFY_SOURCE=2
#for Gtk+ 2
#CFLAGSF= `pkg-config --cflags gtk+-2.0`
#LDFLAGSF= `pkg-config --libs gtk+-2.0`
#for Gtk+ 3
CFLAGSF	= `pkg-config --cflags gtk+-3.0`
LDFLAGSF= `pkg-config --libs gtk+-3.0`
CFLAGS	= -W -Wall -g -O2
LDFLAGS	=
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f
OBJS	= Xdialog.o

all: $(TARGETS)

Xdialog: $(OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Xdialog $(OBJS)

Xdialog.o: Xdialog.c ../src/gbsddialog
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o Xdialog.o -c Xdialog.c

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
