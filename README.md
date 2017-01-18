irssi-tcl is a module for the [Irssi](https://irssi.org/) IRC client. It lets
Irssi support scripting with [Tcl](http://www.tcl.tk/).

Why? Irssi supports Perl scripting in its core. However I am a fan of the Tcl
language, and I've written many IRC related scripts for the
[Eggdrop](http://eggheads.org/) IRC bot. With some effort, you can even update
Eggdrop scripts to run on Irssi.

It also solved a particular problem I had where I wanted to easily use
non-blocking I/O, such as for HTTP requests. Tcl supports this pattern very
easily through its event loop. This makes using Tcl through this module an easy
way to talk to HTTP servers.

The module works by loading a Tcl interpreter into Irssi. You can use th `/tcl`
command to interact with this interpreter. The module also supports loading
scripts.


# Available scripts
I've written a number of scripts using this module. You can find them in
[this repository](https://github.com/horgh/irssi-tcl-scripts).

Another is [irssi-strava](https://github.com/tbalboa/irssi-strava).


# Requirements
To build the module you need:

  * Irssi and Irssi development files
    * Debian packages: irssi irssi-dev
  * glib2 and glib2 development files
    * Debian packages: libglib2.0-0 libglib2.0-dev
  * Tcl 8.5+ and Tcl 8.5+ development files (8.5 and 8.6 both work).
    * Debian packages: tcl8.5-dev tcl8.5
  * GNU Make (gmake)
    * Debian packages: build-essential (well this will get Make among other
      things)

It's possible it will work with older Tcl versions, but I haven't tested this.


# Building
These commands must be run in the `src` directory.

Linux:

    make

FreeBSD:

    gmake -f Makefile.FreeBSD

Didn't work? If building fails, check the included (`-I`) directories listed in
the Makefile actually exist. They may be at a different location on your system
(such as in `/usr/local/include` and `/usr/local/lib`). You will need to alter
them until they are all correct if they are not. Feel free to ask me for help.

Building with debug (which only really alters script/module paths right now):

    make -e DEBUG=-DDEBUG


# Installation
Linux:

    make install

FreeBSD:

    make -f Makefile.FreeBSD install

The install will place `libtcl.so` in `~/.irssi/modules` and Tcl scripts into
`~/.irssi/tcl`.

Alternatively you can manually copy `libtcl.so` where you would like it.
However, the module always looks in `~/.irssi/tcl` for scripts.


# Usage
To load the module in Irssi:

    /load tcl

To autoload the module at startup of Irssi:

    echo "load tcl" >> ~/.irssi/startup

The scripts that are loaded automatically are listed in
`~/.irssi/tcl/scripts.conf`.

Alternatively, a script in `~/.irssi/tcl/` can be loaded via

    /tcl load_script name_of_script.tcl


# Commands
Run a command in the Tcl interpreter:

    /tcl <command ...>

Reload Tcl scripts:

    /tcl reload


# Examples

    /tcl set a [expr 5+5]
    13:53 -!- Tcl: Running /tcl: 'set a [expr 5+5]'
    13:53 -!- Tcl: Result: 10

    /tcl set a
    13:53 -!- Tcl: Running /tcl: 'set a'
    13:53 -!- Tcl: Result: 10

See the "tcl" section in Irssi's `/set` for individual Tcl script settings.


# Scripting
See `docs/scripting.md`.


# Using (edited) Eggdrop scripts
See `docs/eggdrop_scripts.md`.
