TARGETS	= gbsddialog
PREFIX	= /usr/local
BINDIR	= $(PREFIX)/bin
CPPFLAGS= -D_FORTIFY_SOURCE=2
#For compatibility with Xdialog(1)
#CPPFLAGS+= -DWITH_XDIALOG
CFLAGS	= -W -Wall -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS	= `pkg-config --libs gtk+-3.0`
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f
OBJS	= builders.o callbacks.o common.o gbsddialog.o main.o

all: $(TARGETS)

gbsddialog: $(OBJS)
	$(CC) $(LDFLAGS) -o gbsddialog $(OBJS)

builders.o: builders.c builders.h callbacks.h common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o builders.o -c builders.c

callbacks.o: callbacks.c callbacks.h common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o callbacks.o -c callbacks.c

common.o: common.c common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o common.o -c common.c

gbsddialog.o: gbsddialog.c builders.h common.h gbsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o gbsddialog.o -c gbsddialog.c

main.o: main.c common.h gbsddialog.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o main.o -c main.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(BINDIR)
	$(INSTALL) -m 0755 gbsddialog $(BINDIR)/gbsddialog

uninstall:
	$(RM) $(BINDIR)/gbsddialog

.PHONY: all clean distclean install uninstall
