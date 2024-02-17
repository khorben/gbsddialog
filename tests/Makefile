TARGETS	= gbsddialog2 gbsddialog3 Xdialog2 Xdialog3
CPPFLAGS= -D_FORTIFY_SOURCE=2
CFLAGS	= -W -Wall -g -O2 -fstack-protector
LDFLAGS	=
INSTALL	= install
MKDIR	= mkdir -p
RM	= rm -f
OBJS	= gbsddialog2.o gbsddialog3.o Xdialog2.o Xdialog3.o

#for Gtk+ 2
CFLAGSF_GTK2= `pkg-config --cflags gtk+-2.0`
LDFLAGSF_GTK2= `pkg-config --libs gtk+-2.0`

#for Gtk+ 3
CFLAGSF_GTK3= `pkg-config --cflags gtk+-3.0`
LDFLAGSF_GTK3= `pkg-config --libs gtk+-3.0`

all:

tests: $(TARGETS)

gbsddialog2: gbsddialog2.o
	$(CC) $(LDFLAGSF_GTK2) $(LDFLAGS) -o gbsddialog2 gbsddialog2.o

gbsddialog2.o: gbsddialog.c ../src/builders.c ../src/builders.h ../src/callbacks.c ../src/callbacks.h ../src/common.c ../src/common.h ../src/bsddialog.h ../src/gbsddialog.c ../src/gbsddialog.h ../src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGSF_GTK2) $(CFLAGS) -o gbsddialog2.o -c gbsddialog.c

gbsddialog3: gbsddialog3.o
	$(CC) $(LDFLAGSF_GTK3) $(LDFLAGS) -o gbsddialog3 gbsddialog3.o

gbsddialog3.o: gbsddialog.c ../src/builders.c ../src/builders.h ../src/callbacks.c ../src/callbacks.h ../src/common.c ../src/common.h ../src/bsddialog.h ../src/gbsddialog.c ../src/gbsddialog.h ../src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGSF_GTK3) $(CFLAGS) -o gbsddialog3.o -c gbsddialog.c

Xdialog2: Xdialog2.o
	$(CC) $(LDFLAGSF_GTK2) -lm $(LDFLAGS) -o Xdialog2 Xdialog2.o

Xdialog2.o: Xdialog.c ../tools/builders.c ../tools/Xdialog.c
	$(CC) $(CPPFLAGS) $(CFLAGSF_GTK2) $(CFLAGS) -o Xdialog2.o -c Xdialog.c

Xdialog3: Xdialog3.o
	$(CC) $(LDFLAGSF_GTK3) -lm $(LDFLAGS) -o Xdialog3 Xdialog3.o

Xdialog3.o: Xdialog.c ../tools/builders.c ../tools/Xdialog.c
	$(CC) $(CPPFLAGS) `pkg-config --cflags gtk+-3.0` $(CFLAGS) -o Xdialog3.o -c Xdialog.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install:

uninstall:

.PHONY: all tests clean distclean install uninstall
