#!/usr/bin/env tclsh8.5
#
# Use after eggdrop_convert.tcl
#
# Replaces 'putserv $server "PRIVMSG $chan :whatever"'
# with 'putchan $server $chan "whatever"'
#
# Usage:
#  ./eggdrop_convert_putchan.tcl script.tcl > script_converted.tcl
#

if {$argc != 1} {
	puts "Usage: $argv0 <script.tcl>"
	return
}

set f [open [lindex $argv 0]]
set data [read -nonewline $f]
close $f

foreach line [split $data \n] {
	# putserv $server "PRIVMSG $chan :text.."
	# -> putchan $server $chan "text.."
	if {[regexp -- {(\t*?)?putserv \$server "PRIVMSG \$chan :(.*?)"} $line -> tabs text]} {
		puts "${tabs}putchan \$server \$chan \"$text\""

	# ::output_cmd -> putchan
	} elseif {[regexp -- {(\t*?)?(\S*?::output_cmd) \$server "PRIVMSG \$chan :(.*?)"} $line -> tabs cmd text]} {
		puts "${tabs}putchan \$server \$chan \"$text\""
	} else {
		puts $line
	}
}
