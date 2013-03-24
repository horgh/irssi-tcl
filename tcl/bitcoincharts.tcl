#
# 26/2/2013
# will@summercat.com
#
# retrieve and output some markets data from bitcoincharts.com.
#
# http://bitcoincharts.com/about/markets-api/
#

package require http
package require json
# for debugging purposes I host a json dump on a separate server which
# is ssl only, hence this!
package require tls

namespace eval ::bitcoincharts {
	# url to query.
	variable url {http://bitcoincharts.com/t/markets.json}
	#variable url {https://leviathan.summercat.com/~a/code/markets.json}

	# symbols we output.
	#variable symbols [list mtgoxUSD mtgoxCAD mtgoxEUR btceUSD virtexCAD]
	variable symbols [list mtgoxUSD]

	# amount of time to wait between queries. minutes.
	variable query_delay 15

	# last time we queried - cannot query more than every 15 min.
	variable last_query {}

	# cache results of prior queries.
	variable cache {}

	# debug output.
	variable debug 1

	signal_add msg_pub !btc ::bitcoincharts::btc_handler
	signal_add msg_pub .btc ::bitcoincharts::btc_handler

	settings_add_str "bitcoincharts_enabled_channels" ""

	::http::register https 443 ::tls::socket
}

proc ::bitcoincharts::::log {msg} {
	if {!$::bitcoincharts::debug} {
		return
	}
	irssi_print "bitcoincharts: $msg"
}

# @param double $v Value to format
#
# @return double formatted
#
# format a double to less precision
proc ::bitcoincharts::format_double {v} {
	set v [format %.5f $v]
	return $v
}

# @param double $v Value to format
#
# @return double formatted
#
# format a double to less precision and also insert commas
# between every 3 (non decimal) digits.
proc ::bitcoincharts::format_double_thousands {v} {
	set v [::bitcoincharts::format_double $v]
	# break the string on the '.'.
	set number_parts [split $v .]
	set whole_part [lindex $number_parts 0]
	set fractional_part [lindex $number_parts 1]

	# break the whole part into separate digits.
	set whole_part_digits [split $whole_part {}]
	# reverse the digits.
	set whole_part_digits [lreverse $whole_part_digits]

	# take up to 3 digits each time from this reversed list and
	# put these groupings into another list.
	# this is so we can deal with the case of not having full
	# groups of 3.
	set digit_groups [list]
	while {[llength $whole_part_digits]} {
		if {[expr [llength $whole_part_digits] >= 3]} {
			set digit_group [lrange $whole_part_digits 0 2]
			set whole_part_digits [lreplace $whole_part_digits 0 2]
		} else {
			set digit_group [lrange $whole_part_digits 0 end]
			set whole_part_digits [lreplace $whole_part_digits 0 end]
		}
		# reverse the digit group digits as first part of reverting
		# our original reverse.
		set digit_group [lreverse $digit_group]
		# make the digits a string instead of a list
		set digit_group [join $digit_group {}]
		lappend digit_groups $digit_group
	}
	# rebuild the whole part string. we also have to reverse
	# the digit groups as the second step in undoing our reverse.
	set whole_part [join [lreverse $digit_groups] ,]
	# rebuild the full number.
	set full [join [list $whole_part $fractional_part] .]
	return $full
}

# @param dict $data The market data which we use for output
#
# @return void
#
# output market data for all of the symbols we are configured to output for.
proc ::bitcoincharts::output_market_data {server chan data} {
	# we have a list of dicts with symbol data in each dict.
	foreach d $data {
		set symbol [dict get $d symbol]
		# is this a symbol we want to output?
		if {[lsearch -exact $::bitcoincharts::symbols $symbol] == -1} {
			continue
		}

		set high [dict get $d high]
		set latest_trade [dict get $d latest_trade]
		set bid [dict get $d bid]
		set volume [dict get $d volume]
		set currency [dict get $d currency]
		set currency_volume [dict get $d currency_volume]
		set ask [dict get $d ask]
		set close [dict get $d close]
		set avg [dict get $d avg]
		set low [dict get $d low]

		# format the values a bit.
		set latest_trade [clock format $latest_trade]

		set high [::bitcoincharts::format_double $high]
		set bid [::bitcoincharts::format_double $bid]
		set currency_volume [::bitcoincharts::format_double $currency_volume]
		set ask [::bitcoincharts::format_double $ask]
		set close [::bitcoincharts::format_double $close]
		set avg [::bitcoincharts::format_double $avg]
		set low [::bitcoincharts::format_double $low]

		set volume [::bitcoincharts::format_double_thousands $volume]

		#set s "\002Bitcoin Market Data\002 ($symbol) ($currency): High: \$$high Low \$$low Bid: \$$bid Ask: \$$ask Average: \$$avg Close: \$$close Volume: $volume Currency volume: $currency_volume Latest trade: $latest_trade"

		set s "\002Bitcoin\002 ($symbol): \002Last\002: $close ($latest_trade) \002Range\002: ($low - $high) \002Spread\002: ($bid X $ask) \002Volume\002: $volume"
		putchan $server $chan $s
	}
}

# callback for HTTP query for new market data.
proc ::bitcoincharts::get_market_data_cb {server chan token} {
	set data [::http::data $token]
	::http::cleanup $token
	::bitcoincharts::log "get_market_data_cb: in callback"

	# convert to a dict.
	set data [::json::json2dict $data]

	# cache it.
	set ::bitcoincharts::cache $data

	# output.
	::bitcoincharts::output_market_data $server $chan $data
}

# retrieve market data either from a new request or use the cache
# if we requested recently.
proc ::bitcoincharts::get_market_data {server chan} {
	# we may need to use our cache since we do not want to hit the API
	# too often.
	# so chck if have we already queried the API.
	if {$::bitcoincharts::last_query != {}} {
		# we want to know if the next time we can query the API is still
		# in the future.
		set delay_seconds [expr $::bitcoincharts::query_delay * 60]
		set next_query_time [expr $::bitcoincharts::last_query + $delay_seconds]
		set current_time [clock seconds]
		if {[expr $next_query_time > $current_time] } {
			::bitcoincharts::log "get_market_data: using cache"
			::bitcoincharts::output_market_data $server $chan \
				$::bitcoincharts::cache
			return
		}
	}

	set ::bitcoincharts::last_query [clock seconds]
	::bitcoincharts::log "get_market_data: performing new request"
	set token [::http::geturl $::bitcoincharts::url -timeout 60000 \
		-command "::bitcoincharts::get_market_data_cb $server $chan"]
}

# msg_pub signal handler to retrieve and output
# market data.
proc ::bitcoincharts::btc_handler {server nick uhost chan argv} {
	if {![str_in_settings_str "bitcoincharts_enabled_channels" $chan]} {
		return
	}
	::bitcoincharts::get_market_data $server $chan
}

# unit tests for format_double_thousands()
proc ::bitcoincharts::test_format_double_thousands {} {
	set tests [list]
	lappend tests [list 123456789.89 123,456,789.89]
	lappend tests [list 12345678.89 12,345,678.89]
	lappend tests [list 1234567.89 1,234,567.89]
	lappend tests [list 123456.89 123,456.89]
	lappend tests [list 12345.89 12,345.89]
	lappend tests [list 1234.89 1,234.89]
	lappend tests [list 123.89 123.89]
	lappend tests [list 12.89 12.89]
	lappend tests [list 1.89 1.89]
	lappend tests [list 0.89 0.89]
	lappend tests [list 0.8 0.8]

	foreach test $tests {
		lassign $test value wanted
		set r [::bitcoincharts::format_double_thousands $value]
		if {$r != $wanted} {
			puts "failure: $value should have given $wanted, but gave $r"
		}
		puts "success: $value gave $r"
	}
}

irssi_print "bitcoincharts.tcl loaded"
