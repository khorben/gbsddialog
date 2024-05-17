TARGETS	= $(OBJDIR)gbsddialog2 $(OBJDIR)gbsddialog3 $(OBJDIR)Xdialog2 $(OBJDIR)Xdialog3
CPPFLAGS= -D_FORTIFY_SOURCE=2
CFLAGS	= -W -Wall -g -O2 -fstack-protector
LDFLAGSF=
LDFLAGS	= -lm
RM	= rm -f
OBJS	= $(OBJDIR)gbsddialog2.o $(OBJDIR)gbsddialog3.o $(OBJDIR)Xdialog2.o $(OBJDIR)Xdialog3.o

#for Gtk+ 2
CFLAGSF_GTK2= `pkg-config --cflags gtk+-2.0`
LDFLAGSF_GTK2= `pkg-config --libs gtk+-2.0`

#for Gtk+ 3
CFLAGSF_GTK3= `pkg-config --cflags gtk+-3.0`
LDFLAGSF_GTK3= `pkg-config --libs gtk+-3.0`

all:

tests: $(TARGETS)

$(OBJDIR)gbsddialog2: $(OBJDIR)gbsddialog2.o
	$(CC) $(LDFLAGSF) $(LDFLAGSF_GTK2) $(LDFLAGS) -o $(OBJDIR)gbsddialog2 $(OBJDIR)gbsddialog2.o

$(OBJDIR)gbsddialog2.o: gbsddialog.c ../src/builders.c ../src/builders.h ../src/callbacks.c ../src/callbacks.h ../src/common.c ../src/common.h ../src/bsddialog.h ../src/gbsddialog.c ../src/gbsddialog.h ../src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGSF_GTK2) $(CFLAGS) -o $(OBJDIR)gbsddialog2.o -c gbsddialog.c

$(OBJDIR)gbsddialog3: $(OBJDIR)gbsddialog3.o
	$(CC) $(LDFLAGSF) $(LDFLAGSF_GTK3) $(LDFLAGS) -o $(OBJDIR)gbsddialog3 $(OBJDIR)gbsddialog3.o

$(OBJDIR)gbsddialog3.o: gbsddialog.c ../src/builders.c ../src/builders.h ../src/callbacks.c ../src/callbacks.h ../src/common.c ../src/common.h ../src/bsddialog.h ../src/gbsddialog.c ../src/gbsddialog.h ../src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGSF_GTK3) $(CFLAGS) -o $(OBJDIR)gbsddialog3.o -c gbsddialog.c

$(OBJDIR)Xdialog2: $(OBJDIR)Xdialog2.o
	$(CC) $(LDFLAGSF) $(LDFLAGSF_GTK2) $(LDFLAGS) -o $(OBJDIR)Xdialog2 $(OBJDIR)Xdialog2.o

$(OBJDIR)Xdialog2.o: Xdialog.c ../tools/builders.c ../tools/Xdialog.c
	$(CC) $(CPPFLAGS) $(CFLAGSF_GTK2) $(CFLAGS) -o $(OBJDIR)Xdialog2.o -c Xdialog.c

$(OBJDIR)Xdialog3: $(OBJDIR)Xdialog3.o
	$(CC) $(LDFLAGSF) $(LDFLAGSF_GTK3) $(LDFLAGS) -o $(OBJDIR)Xdialog3 $(OBJDIR)Xdialog3.o

$(OBJDIR)Xdialog3.o: Xdialog.c ../tools/builders.c ../tools/Xdialog.c
	$(CC) $(CPPFLAGS) `pkg-config --cflags gtk+-3.0` $(CFLAGS) -o $(OBJDIR)Xdialog3.o -c Xdialog.c

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGETS)

install:

uninstall:

.PHONY: all tests clean distclean install uninstall
