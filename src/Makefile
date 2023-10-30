TARGETS	= gbsddialog
CPPFLAGS= -D_FORTIFY_SOURCE=2
CFLAGS	= -W -Wall -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS	= `pkg-config --libs gtk+-3.0`

gbsddialog: main.o
	$(CC) $(LDFLAGS) -o gbsddialog main.o

main.o: main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o main.o -c main.c
