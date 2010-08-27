CC=gcc
CFLAGS=-shared -Wall
DFLAGS=-DHAVE_CONFIG_H
LINKS =-ltcl8.5
INCLUDES =-I/usr/include/tcl8.5 -I/usr/include/irssi -I/usr/include/irssi/src -I/usr/include/irssi/src/core -I/usr/include/irssi/src/fe-common/core -I/usr/include/irssi/src/irc/core -I. -I/usr/include/glib-2.0 -I/usr/local/include/glib-2.0/glib -I/usr/lib/glib-2.0 -I/usr/lib/glib-2.0/include

all: libtcl.so

libtcl.so: tcl_module.c tcl_module.h
	$(CC) $(LINKS) $(INCLUDES) $(CFLAGS) -o $@ $< $(DFLAGS)

install: libtcl.so
	mkdir -p ~/.irssi/modules
	cp libtcl.so ~/.irssi/modules
	mkdir -p ~/.irssi/tcl
	cp ./tcl/* ~/.irssi/tcl

clean:
	rm libtcl.so
