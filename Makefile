# Makefile for aprs temperature reader
CC=gcc
CFLAGS=-Wall

all: hamds18b20

hamds18b20: hamds18b20.c
	$(CC) $(CFLAGS) hamds18b20.c -o hamds18b20

clean:
	rm hamds18b20

install:
	install hamds18b20 /usr/bin
	install hamds18b20_eq /usr/bin
	install hamds18b20_labels /usr/bin
	install hamds18b20_status /usr/bin
	install hamds18b20_telem /usr/bin
	install hamds18b20_units /usr/bin
	install hamds18b20.conf /etc/hamds18b20.conf

uninstall:
	rm -f /usr/bin/hamds18b20*
	rm -f /etc/hamds18b20.conf
