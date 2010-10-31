#
# slang.tcl - June 24 2010
# by horgh
#
# Requires Tcl 8.5+ and tcllib
#
# Made with heavy inspiration from perpleXa's urbandict script!
#
# Must .chanset #channel +ud
#
# Uses is.gd to shorten long definition URL if isgd.tcl package present
#

package require htmlparse
package require http

namespace eval ud {
	# set this to !ud or whatever you want
	variable trigger "slang"

	# maximum lines to output
	variable max_lines 1

	# approximate characters per line
	variable line_length 400

	# show truncated message / url if more than one line
	variable show_truncate 1

	variable client "Mozilla/5.0 (compatible; Y!J; for robot study; keyoshid)"
	variable url http://www.urbandictionary.com/define.php
	variable list_regexp {<td class='text'.*? id='entry_.*?'>.*?</td>}
	variable def_regexp {id='entry_(.*?)'>.*?<div class="definition">(.*?)</div>}

	settings_add_str "slang_enabled_channels" ""
	signal_add msg_pub $ud::trigger ud::handler

	# 0 if isgd package is present
	variable isgd_disabled [catch {package require isgd}]
}

proc ud::handler {server nick uhost chan argv} {
	if {![str_in_settings_str "slang_enabled_channels" $chan]} {
		return
	}

	set argv [split $argv]
	if {[string is digit [lindex $argv 0]]} {
		set number [lindex $argv 0]
		set query [join [lrange $argv 1 end]]
	} else {
		set query [join $argv]
		set number 1
	}

	if {$query == ""} {
		putchan $server $chan "Usage: $ud::trigger \[#\] <definition to look up>"
		return
	}

	ud::fetch $query $number $server $chan
}

proc ud::fetch {query number server channel} {
	http::config -useragent $ud::client
	set page [expr {int(ceil($number / 7.0))}]
	set number [expr {$number - (($page - 1) * 7)}]

	set http_query [http::formatQuery term $query page $page]

	set token [http::geturl $ud::url -timeout 20000 -query $http_query -command "ud::output $server $channel [list $query] $number"]
}

# Callback from HTTP get in ud::fetch
proc ud::output {server channel query number token} {
	set data [http::data $token]
	set ncode [http::ncode $token]
	http::cleanup $token

	if {$ncode != 200} {
		putchan $server $channel "HTTP fetch error. Code: $ncode"
		return
	}

	set definitions [regexp -all -inline -- $ud::list_regexp $data]
	if {[llength $definitions] < $number} {
		putchan $server $channel "Error: [llength $definitions] definitions found."
		return
	}

	set result [ud::parse $query [lindex $definitions [expr {$number - 1}]]]

	foreach line [ud::split_line $ud::line_length [dict get $result definition]] {
		if {[incr output] > $ud::max_lines} {
			if {$ud::show_truncate} {
				putchan $server $channel "Output truncated. [ud::def_url $query $result]"
			}
			break
		}
		putchan $server $channel "$line"
	}
}

proc ud::parse {query raw_definition} {
	if {![regexp $ud::def_regexp $raw_definition -> number definition]} {
		error "Could not parse HTML"
	}
	set definition [htmlparse::mapEscapes $definition]
	set definition [regsub -all -- {<.*?>} $definition ""]
	set definition [regsub -all -- {\n+} $definition " "]
	set definition [string tolower $definition]
	return [list number $number definition "$query is $definition"]
}

proc ud::def_url {query result} {
	set raw_url ${ud::url}?[http::formatQuery term $query defid [dict get $result number]]
	if {$ud::isgd_disabled} {
		return $raw_url
	} else {
		if {[catch {isgd::shorten $raw_url} shortened]} {
			return "$raw_url (is.gd error)"
		} else {
			return $shortened
		}
	}
}

# by fedex
proc ud::split_line {max str} {
	set last [expr {[string length $str] -1}]
	set start 0
	set end [expr {$max -1}]

	set lines []

	while {$start <= $last} {
		if {$last >= $end} {
			set end [string last { } $str $end]
		}

		lappend lines [string trim [string range $str $start $end]]
		set start $end
		set end [expr {$start + $max}]
	}

	return $lines
}

irssi_print "slang.tcl loaded"
