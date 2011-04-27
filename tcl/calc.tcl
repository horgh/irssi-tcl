#
# created by fedex
# updated by horgh
#

# Part of tcllib
package require math

namespace eval calc {
	signal_add msg_pub !calc calc::safe_calc
	signal_add msg_pub .calc calc::safe_calc

	settings_add_str "calc_enabled_channels" ""
}

proc ::tcl::mathfunc::factorial {n} {
	#return [math::factorial [expr {int($n)}]]
	return [math::factorial $n]
}

proc calc::is_op {str} {
	return [expr [lsearch {{ } . + - * / ( ) %} $str] != -1]
}

proc calc::safe_calc {server nick uhost chan str} {
	if {![str_in_settings_str "calc_enabled_channels" $chan]} {
		return
	}

	# treat ^ as exponentiation
	regsub -all -- {\^} $str "**" str

	foreach char [split $str {}] {
		# allow characters so as to be able to call mathfuncs
		if {![is_op $char] && ![regexp -- {^[a-z0-9]$} $char]} {
			putchan $server $chan "${nick}: Invalid expression. (${str})"
			return
		}
	}

	# make all values floating point
	set str [regsub -all -- {((?:\d+)?\.?\d+)} $str {[expr {\1*1.0}]}]
	set str [subst $str]

	if {[catch {expr $str} out]} {
		putchan $server $chan "${nick}: Invalid equation. (${str})"
		return
	} else {
		putchan $server $chan "$str = $out"
	}
}

irssi_print "calc.tcl loaded"
