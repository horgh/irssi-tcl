TCL_DIR=/usr/include/tcl8.5
IRSSI_DIR=/usr/include/irssi
GLIB2_DIR=/usr/include/glib-2.0
GLIB2_DIR_2=/usr/lib/glib-2.0
LIB_DIR=/usr/lib
TCL_LIB=tcl8.5

TCL_INCLUDES=-I$(TCL_DIR)
IRSSI_INCLUDES=-I$(IRSSI_DIR) -I$(IRSSI_DIR)/src -I$(IRSSI_DIR)/src/core -I$(IRSSI_DIR)/src/fe-common/core -I$(IRSSI_DIR)/src/irc/core
GLIB2_INCLUDES=-I$(GLIB2_DIR) -I$(GLIB2_DIR)/glib -I$(GLIB2_DIR_2) -I$(GLIB2_DIR_2)/include

CC=gcc
CFLAGS=-shared -Wall
DFLAGS=-DHAVE_CONFIG_H
LINKS=-l$(TCL_LIB) -L$(LIB_DIR)
INCLUDES=-I. $(TCL_INCLUDES) $(IRSSI_INCLUDES) $(GLIB2_INCLUDES)

SRC=module.c
OBJS=signals.o tcl_commands.o

all: libtcl.so

libtcl.so: module.c module.h $(OBJS)
	$(CC) $(LINKS) $(INCLUDES) $(CFLAGS) $(OBJS) -o $@ $(SRC) $(DFLAGS)

%.o: %.c %.h
	$(CC) $(INCLUDES) $(CFLAGS) -c $(DFLAGS) -o $@ $<

install: libtcl.so
	mkdir -p $(HOME)/.irssi/modules
	cp libtcl.so $(HOME)/.irssi/modules
	mkdir -p $(HOME)/.irssi/tcl
	cp ../scripts/* $(HOME)/.irssi/tcl

clean:
	rm -f libtcl.so $(OBJS)
