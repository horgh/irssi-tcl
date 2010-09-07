#
# created by horgh
#
# Provides some binds to read Yahoo.com futures
#

package require http

namespace eval latoc {
	variable user_agent "Lynx/2.8.5rel.1 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/0.9.7e"

	variable list_regexp {<tr><td class="first">.*?<td class="last">.*?</td></tr>}
	variable stock_regexp {<a href="/q\?s=(.*?)">.*?<td class="second name">(.*?)</td>.*?<span id=".*?">(.*?)</span></b> <nobr><span id=".*?">(.*?)</span></nobr>.*?(?:alt="(.*?)">)?? <b style="color.*?;">(.*?)</b>.*?<b style="color.*?;"> \((.*?)\)</b>}

	# any names matching this pattern are not shown
	variable skip_regexp {(5000 oz)|(100 oz)}

	variable commodities [list energy metals grains livestock softs]
	variable energy_futures "http://finance.yahoo.com/futures?t=energy"
	variable commodities_url "http://finance.yahoo.com/futures?t="

	signal_add msg_pub "!oil" latoc::oil_handler
	signal_add msg_pub "!gold" latoc::gold_handler
	signal_add msg_pub "!c" latoc::commodity_handler
	signal_add msg_pub "!silver" latoc::silver_handler
	signal_add msg_pub "!url" latoc::url_handler

	settings_add_str "latoc_enabled_channels" ""
}

proc latoc::url_handler {server nick uhost chan argv} {
	putchan $server $chan "$latoc::commodities_url"
}

# fetch lines from given commodity type (url) and only return lines that
# match the given pattern (regexp) to Name (optional)
# return list of lines, each a stock
proc latoc::fetch {type {pattern {}}} {
	set token [http::geturl ${latoc::commodities_url}${type} -timeout 60000]
	set data [http::data $token]
	set ncode [http::ncode $token]
	http::cleanup $token

	if {$ncode != 200} {
		error "HTTP error: (code: $ncode): $data"
	}

	set lines []
	foreach stock [regexp -all -inline -- $latoc::list_regexp $data] {
		regexp $latoc::stock_regexp $stock -> symbol name price last direction change percent
		if {[regexp -- $pattern $name]} {
			if {[regexp -- $latoc::skip_regexp $name]} {
				continue
			}
			lappend lines [latoc::format $name $price $last $direction $change $percent]
		}
	}

	if {[llength $lines] == 0} {
		lappend lines "No results."
	}
	return $lines
}

proc latoc::commodity_handler {server nick uhost chan argv} {
	if {![channel_in_settings_str "latoc_enabled_channels" $chan]} { return }
	if {[lsearch $latoc::commodities $argv] == -1} {
		putchan $server $chan "Valid commodities are: $latoc::commodities"
		return
	}

	if {[catch {latoc::fetch $argv} result]} {
		putchan $server $chan "Error: $result"
		return
	}

	foreach line $result {
		putchan $server $chan "$line"
	}
}

proc latoc::oil_handler {server nick uhost chan argv} {
	if {![channel_in_settings_str "latoc_enabled_channels" $chan]} { return }

	if {[catch {latoc::fetch "energy" "Crude Oil"} result]} {
		putchan $server $chan "Error: $result"
		return
	}

	foreach line $result {
		putchan $server $chan "$line"
	}
}

proc latoc::gold_handler {server nick uhost chan argv} {
	if {![channel_in_settings_str "latoc_enabled_channels" $chan]} { return }

	if {[catch {latoc::fetch "metals" "Gold"} result]} {
		putchan $server $chan "Error: $result"
		return
	}

	foreach line $result {
		putchan $server $chan "$line"
	}
}

proc latoc::silver_handler {server nick uhost chan argv} {
	if {![channel_in_settings_str "latoc_enabled_channels" $chan]} { return }

	if {[catch {latoc::fetch "metals" "Silver"} result]} {
		putchan $server $chan "Error: $result"
		return
	}

	foreach line $result {
		putchan $server $chan "$line"
	}
}

proc latoc::format {name price last direction change percent} {
# this cuts off the Jun 09 part from Crude Oil Jun 09
#	set name [lrange $name 0 [expr [llength $name]-3]]
	return "$name: \00310$price [latoc::colour $direction $change] [latoc::colour $direction $percent]\003 $last"
}

proc latoc::colour {direction value} {
	if {[string match "Down" $direction]} {
		return \00304-$value\017
	} elseif {[string match "Up" $direction]} {
		return \00309+$value\017
	} else {
		return $value
	}
}

irssi_print "latoc.tcl loaded"
