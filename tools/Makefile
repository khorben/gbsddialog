TARGETS	= Xdialog
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
CPPFLAGS= -D_FORTIFY_SOURCE=2
CFLAGSF	= `pkg-config --cflags gtk+-3.0`
CFLAGS	= -W -Wall -g -O2 -fstack-protector
LDFLAGSF= `pkg-config --libs gtk+-3.0` -lm
LDFLAGS	=
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f
OBJS	= builders.o Xdialog.o

#uncomment the following two lines to build with Gtk+ 2:
#CFLAGSF= `pkg-config --cflags gtk+-2.0`
#LDFLAGSF= `pkg-config --libs gtk+-2.0`

all: $(TARGETS)

Xdialog: $(OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o Xdialog $(OBJS)

builders.o: builders.c ../src/builders.c ../src/builders.h ../src/callbacks.c ../src/common.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o builders.o -c builders.c

Xdialog.o: Xdialog.c ../src/builders.h ../src/common.c ../src/common.h ../src/gbsddialog.c ../src/gbsddialog.h ../src/main.c
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
