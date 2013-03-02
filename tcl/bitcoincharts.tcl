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
	#variable url {https://leviathan.summercat.com/~a/btc.json}

	# symbols we output.
	#variable symbols [list mtgoxUSD mtgoxCAD mtgoxEUR btceUSD virtexCAD]
	variable symbols [list mtgoxUSD mtgoxCAD]

	# amount of time to wait between queries. minutes.
	variable query_delay 30

	# last time we queried - cannot query more than every 15 min.
	variable last_query {}

	# cache results of prior queries.
	variable cache {}

	# debug output.
	variable debug 1

	signal_add msg_pub !btc ::bitcoincharts::btc_handler
	signal_add msg_pub btc ::bitcoincharts::btc_handler
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

# @param double $v
#
# @return double $v
#
# format a double
proc ::bitcoincharts::format_double {v} {
	set v [format %.5f $v]
	return $v
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

		# format these a bit.
		set latest_trade [clock format $latest_trade]
		set high [::bitcoincharts::format_double $high]
		set bid [::bitcoincharts::format_double $bid]
		set volume [::bitcoincharts::format_double $volume]
		set currency_volume [::bitcoincharts::format_double $currency_volume]
		set ask [::bitcoincharts::format_double $ask]
		set close [::bitcoincharts::format_double $close]
		set avg [::bitcoincharts::format_double $avg]
		set low [::bitcoincharts::format_double $low]

		set s "\002Bitcoin Market Data\002 ($symbol) ($currency): High: \$$high Low \$$low Bid: \$$bid Ask: \$$ask Average: \$$avg Close: \$$close Volume: $volume Currency volume: $currency_volume Latest trade: $latest_trade"
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
	bitcoincharts::get_market_data $server $chan
}

irssi_print "bitcoincharts.tcl loaded"
