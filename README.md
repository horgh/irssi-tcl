# 1. Requirements

To build/use the module you need:

  * Irssi and Irssi development files
  * glib2 and glib2 development files
  * Tcl 8.5+ and Tcl 8.5+ development files
  * GNU Make (gmake)

It's possible it will work with older Tcl versions, but I haven't tested. Nor
have I tested with higher than 8.5.x actually.


# 2. Building

Linux:

    make

FreeBSD:

    gmake -f Makefile.FreeBSD

Didn't work?
If building fails, check the included (-I) directories listed in the Makefile
actually exist.
They may be at a different location on your system (such as in
/usr/local/include and /usr/local/lib).
You will need to alter them until they are all correct if they are not.

Building with debug (only really alters script/module paths right now):

    make -e DEBUG=-DDEBUG


# 3. Installing

Linux:

    make install

FreeBSD:

    make -f Makefile.FreeBSD install

The install will place libtcl.so in ~/.irssi/modules and Tcl scripts into
~/.irssi/tcl

Alternatively you can manually copy libtcl.so where you would like it, but the
module will look in ~/.irssi/tcl for scripts.


# 4. Usage

To load the module in Irssi:

    /load tcl

To autoload the module at startup of Irssi:

    echo "load tcl" >> ~/.irssi/startup

The scripts that are loaded automatically are listed in
~/.irssi/tcl/scripts.conf.

Alternatively, a script in ~/.irssi/tcl/ can be loaded via

    /tcl load_script name_of_script.tcl


# 5. Commands

Run a command in the Tcl interpreter:

    /tcl <command ...>

Reload Tcl scripts:

    /tcl reload


# 6. Examples

    /tcl set a [expr 5+5]
    13:53 -!- Tcl: Running /tcl: 'set a [expr 5+5]'
    13:53 -!- Tcl: Result: 10

    /tcl set a
    13:53 -!- Tcl: Running /tcl: 'set a'
    13:53 -!- Tcl: Result: 10

See the "tcl" section in /set for Tcl script settings.


# 7. Scripting

See docs/scripting.md.


# 8. Using (edited) eggdrop scripts

See docs/eggdrop\_scripts.md.


# 9. Sample scripts

There are some sample scripts for this module in the repository
'irssi-tcl-scripts'.
