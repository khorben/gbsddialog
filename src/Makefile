TARGETS	= gbsddialog
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
CPPFLAGS= -D_FORTIFY_SOURCE=2
CFLAGSF	= `pkg-config --cflags gtk+-3.0`
CFLAGS	= -W -Wall -g -O2 -fstack-protector
LDFLAGSF= `pkg-config --libs gtk+-3.0`
LDFLAGS	=
INSTALL	= install
MKDIR	= mkdir -p -m 0755
RM	= rm -f
OBJS	= builders.o callbacks.o common.o gbsddialog.o main.o

#uncomment the following two lines to build with Gtk+ 2:
#CFLAGSF= `pkg-config --cflags gtk+-2.0`
#LDFLAGSF= `pkg-config --libs gtk+-2.0`

all: $(TARGETS)

gbsddialog: $(OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o gbsddialog $(OBJS)

builders.o: builders.c builders.h callbacks.h common.h bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o builders.o -c builders.c

callbacks.o: callbacks.c callbacks.h common.h bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o callbacks.o -c callbacks.c

common.o: common.c common.h bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o common.o -c common.c

gbsddialog.o: gbsddialog.c builders.h common.h bsddialog.h gbsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o gbsddialog.o -c gbsddialog.c

main.o: main.c common.h bsddialog.h gbsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o main.o -c main.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 gbsddialog $(DESTDIR)$(BINDIR)/gbsddialog

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/gbsddialog

.PHONY: all clean distclean install uninstall
