#!/usr/bin/env tclsh8.5
#
# Attempt to convert eggdrop Tcl script to Irssi Tcl script
#
# Usage:
#  ./eggdrop_convert.tcl eggdrop_script.tcl > irssi_script.tcl
#

if {$argc != 1} {
	puts "Usage: $argv0 <eggdrop script.tcl>"
	return
}

set f [open [lindex $argv 0]]
set data [read -nonewline $f]
close $f

foreach line [split $data \n] {
	if {[regexp -- {proc.* hand } $line]} {
		set line [regsub -all -- { hand } $line " "]
		set line [regsub -- {\{} $line "\{server "]
	# I use variable output_cmd putserv in some scripts, don't replace that
	} elseif {[regexp -- {^[^(variable)]putserv} $line]} {
		set line [regsub -- {putserv} $line {putserv $server}]
	# Deal with this case of mine
	} elseif {[regexp -- {::output_cmd} $line]} {
		set line [regsub -- {::output_cmd} $line {::output_cmd $server}]
	}
	puts $line
}
