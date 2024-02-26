TARGETS	= $(OBJDIR)gbsddialog
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
OBJS	= $(OBJDIR)builders.o $(OBJDIR)callbacks.o $(OBJDIR)common.o $(OBJDIR)gbsddialog.o $(OBJDIR)main.o

#uncomment the following two lines to build with Gtk+ 2:
#CFLAGSF= `pkg-config --cflags gtk+-2.0`
#LDFLAGSF= `pkg-config --libs gtk+-2.0`

all: $(TARGETS)

$(OBJDIR)gbsddialog: $(OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $(OBJDIR)gbsddialog $(OBJS)

$(OBJDIR)builders.o: builders.c builders.h callbacks.h common.h bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)builders.o -c builders.c

$(OBJDIR)callbacks.o: callbacks.c callbacks.h common.h bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)callbacks.o -c callbacks.c

$(OBJDIR)common.o: common.c common.h bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)common.o -c common.c

$(OBJDIR)gbsddialog.o: gbsddialog.c builders.h common.h bsddialog.h gbsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)gbsddialog.o -c gbsddialog.c

$(OBJDIR)main.o: main.c common.h bsddialog.h gbsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)main.o -c main.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)gbsddialog $(DESTDIR)$(BINDIR)/gbsddialog

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/gbsddialog

.PHONY: all clean distclean install uninstall
