#
# vim: tabstop=2:shiftwidth=2:noexpandtab
#
# see README for compiling with debugging
#

TCL_DIR=/usr/include/tcl8.6

# To compile against a git checkout of irssi, change this value.
# e.g.,
#IRSSI_DIR=/home/will/bin/irssi.git
IRSSI_DIR=/usr/include/irssi

LIB_DIR=/usr/lib
TCL_LIB=tcl8.6

GLIB2_INCLUDES=`pkg-config --cflags glib-2.0`

TCL_INCLUDES=-I$(TCL_DIR)

IRSSI_INCLUDES= \
	-I$(IRSSI_DIR) -I$(IRSSI_DIR)/src -I$(IRSSI_DIR)/src/core \
	-I$(IRSSI_DIR)/src/fe-common/core -I$(IRSSI_DIR)/src/irc/core \

CC=gcc

# NOTE: -Wpadded generates from includes!
CFLAGS = \
	-shared \
	-std=c11 -O1 -g -ggdb -pedantic -pedantic-errors \
	-Werror -Wall -Wextra \
	-Wformat-y2k -Wformat-nonliteral -Wformat-security \
	-Wformat=2 \
	-Winit-self \
	-Wmissing-include-dirs -Wswitch-default \
	-Wunused-local-typedefs \
	-Wunused-parameter \
	-Wunused-result \
	-Wunused \
	-Wuninitialized \
	-Wfloat-equal -Wundef -Wshadow -Wpointer-arith \
	-Wbad-function-cast \
	-Wempty-body \
	-Wsign-conversion -Wlogical-op -Waggregate-return \
	-Wcast-align -Wstrict-prototypes -Wmissing-prototypes \
	-Wold-style-definition -Wpacked \
	-Wredundant-decls \
	-Wnested-externs -Winline -Winvalid-pch \
	-Wwrite-strings -Waggregate-return \
	-Wswitch-enum -Wconversion -Wunreachable-code

LINKS=-l$(TCL_LIB) -L$(LIB_DIR)
INCLUDES=-I. $(TCL_INCLUDES) $(IRSSI_INCLUDES) $(GLIB2_INCLUDES)

OBJS=irssi.o irssi_commands.o module.o signals.o tcl_commands.o tcl_core.o
TARGETS=libtcl.so

all: $(TARGETS)

libtcl.so: $(OBJS)
	$(CC) $(INCLUDES) $(CFLAGS) $(OBJS) $(LINKS) -o $@ $(DEBUG)

%.o: %.c %.h Makefile irssi_includes.h debug.h
	$(CC) $(INCLUDES) $(CFLAGS) -fPIC -c -o $@ $< $(DEBUG)

install: libtcl.so
	mkdir -p $(HOME)/.irssi/modules
	cp libtcl.so $(HOME)/.irssi/modules
	mkdir -p $(HOME)/.irssi/tcl
	cp ../tcl/*.tcl $(HOME)/.irssi/tcl
	if test -e $(HOME)/.irssi/tcl/scripts.conf; \
	then echo "Not replacing existing scripts.conf"; \
	else cp ../tcl/scripts.conf $(HOME)/.irssi/tcl; \
	fi

clean:
	rm -f $(TARGETS) $(OBJS)
