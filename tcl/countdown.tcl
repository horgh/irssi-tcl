#

namespace eval countdown {
	signal_add msg_pub .countdown countdown::countdown
	signal_add msg_pub .cd countdown::countdown

	settings_add_str "countdown_enabled_channels" ""

	variable timer 0
}

proc countdown::countdown {server nick uhost chan str} {
	if {![str_in_settings_str "countdown_enabled_channels" $chan]} { return }

	# already running
	if {$countdown::timer} { return }

	set args [split $str]
	if {[llength $args] != 1 || ![string is integer [lindex $args 0]]} {
		putchan $server $chan "Usage: .countdown <time in seconds>"
		return
	}
	set seconds [lindex $args 0]

	if {$seconds < 3 || $seconds > 10} {
		putchan $server $chan "Valid times: 3 <= time <= 10."
		return
	}

	set countdown::timer 1
	putchan $server $chan "Countdown starting!"
	set t 1000
	for {set i $seconds} {$i >= 0} {incr i -1} {
		if {$i == 0} {
			after $t [list countdown::print $server $chan "GO!"]
			after $t [list set countdown::timer 0]
			break
		}
		after $t [list countdown::print $server $chan "$i..."]
		incr t 1000
	}
}

proc countdown::print {server chan str} {
	putchan $server $chan $str
}

irssi_print "countdown.tcl loaded"
