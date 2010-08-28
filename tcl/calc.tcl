#
# created by fedex
# updated by horgh
#

namespace eval calc {
	signal_add msg_pub !calc calc::safe_calc
	signal_add msg_pub .calc calc::safe_calc

	settings_add_str "calc_enabled_channels" ""
}

proc calc::is_op {str} {
	return [expr [lsearch {{ } . + - * / ( ) %} $str] != -1]
}

proc calc::safe_calc {server nick uhost chan str} {
	if {![channel_in_settings_str "calc_enabled_channels" $chan]} {
		return
	}

	foreach char [split $str {}] {
		if {![is_op $char] && ![string is integer $char]} {
			putchan $server $chan "$nick: Invalid expression for calc."
			return
		}
	}

	# make all values floating point
	set str [regsub -all -- {((?:\d+)?\.?\d+)} $str {[expr {\1*1.0}]}]
	set str [subst $str]

	if {[catch {expr $str} out]} {
		putchan $server $chan "$nick: Invalid equation."
		return
	} else {
		putchan $server $chan "$str = $out"
	}
}

irssi_print "calc.tcl loaded"
