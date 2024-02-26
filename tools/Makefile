TARGETS	= $(OBJDIR)Xdialog
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
CPPFLAGS= -D_FORTIFY_SOURCE=2
CFLAGSF	= `pkg-config --cflags gtk+-3.0`
CFLAGS	= -W -Wall -g -O2 -fstack-protector
LDFLAGSF= `pkg-config --libs gtk+-3.0` -lm
LDFLAGS	=
INSTALL	= install
MKDIR	= mkdir -p -m 0755
RM	= rm -f
OBJS	= $(OBJDIR)builders.o $(OBJDIR)Xdialog.o

#uncomment the following two lines to build with Gtk+ 2:
#CFLAGSF= `pkg-config --cflags gtk+-2.0`
#LDFLAGSF= `pkg-config --libs gtk+-2.0`

all: $(TARGETS)

$(OBJDIR)Xdialog: $(OBJS)
	$(CC) $(LDFLAGSF) $(LDFLAGS) -o $(OBJDIR)Xdialog $(OBJS)

$(OBJDIR)builders.o: builders.c ../src/builders.c ../src/builders.h ../src/callbacks.c ../src/common.h ../src/bsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)builders.o -c builders.c

$(OBJDIR)Xdialog.o: Xdialog.c ../src/builders.h ../src/common.c ../src/common.h ../src/bsddialog.h ../src/gbsddialog.c ../src/gbsddialog.h ../src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)Xdialog.o -c Xdialog.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)Xdialog $(DESTDIR)$(BINDIR)/Xdialog

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/Xdialog

.PHONY: all clean distclean install uninstall
