#
# 2010-10-02
#

namespace eval nano {
	signal_add msg_pub .nano nano::nano
	settings_add_str "nano_enabled_channels" ""
}

proc nano::nano {server nick uhost chan str} {
	if {![str_in_settings_str "nano_enabled_channels" $chan]} { return }
	set day_raw [clock format [clock seconds] -format %d]
	regexp -- {0?(\d+)} $day_raw -> day
	set expected_words [expr {ceil( 50000/30 * $day)}]
	putchan $server $chan $expected_words
}

irssi_print "nanowrimo.tcl loaded"
