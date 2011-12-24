#
# 0.3 - ?
#  - switch from decode_html to htmlparse::mapEscape
#  - fix issue with encoding getting ascii
#  - add !g1 for one result
#  - strip remaining html from api result
#
# 0.2 - May 10 2010
#  - fix for garbled utf chars in api queries
#  - added +google channel flag to enable
#  - strip html from !convert as some formatting may be present
#  - fix decode_html to convert html utf to hex
#  - convert <sup></sup> to exponent
#
# 0.1 - Some time in April 2010
#  - Initial release
#
# Created Feb 28 2010
#
# Requires Tcl 8.5+
# Requires tcllib for json
#

package require http
package require json
package require htmlparse

namespace eval google {
	variable useragent_api "Lynx/2.8.8dev.2 libwww-FM/2.14 SSL-MM/1.4.1"
	variable useragent_convert "Mozilla/5.0 (X11; Linux i686; rv:8.0) Gecko/20100101 Firefox/8.0"

	variable convert_url "http://www.google.ca/search"
	variable convert_regexp {<table class=std>.*?<b>(.*?)</b>.*?</table>}

	variable api_url "http://ajax.googleapis.com/ajax/services/search/"

	variable api_referer "http://www.egghelp.org"

	signal_add msg_pub "!g" google::search
	signal_add msg_pub "!google" google::search
	signal_add msg_pub "!g1" google::search1
	signal_add msg_pub "!news" google::news
	signal_add msg_pub "!images" google::images
	signal_add msg_pub "!convert" google::convert

	settings_add_str "google_enabled_channels" ""
}

# Query normal html for conversions
proc google::convert {server nick uhost chan argv} {
	if {![str_in_settings_str google_enabled_channels $chan]} { return }

	if {[string length $argv] == 0} {
		putchan $server $chan "Please provide a query."
		return
	}

	http::config -useragent $google::useragent_convert
	set query [http::formatQuery q $argv]
	set token [http::geturl ${google::convert_url}?${query} -command "google::convert_callback $server $chan"]
}

proc google::convert_callback {server chan token} {
	set data [http::data $token]
	set ncode [http::ncode $token]
	http::cleanup $token

	# debug
	#set fid [open "g-debug.txt" w]
	#puts $fid $data
	#close $fid

	if {$ncode != 200} {
		putchan $server $chan "HTTP query failed: $ncode"
		return
	}

	if {[catch {google::convert_parse $data} result]} {
		putchan $server $chan "Error: $result."
		return
	}

	putchan $server $chan "\002$result\002"
}

proc google::convert_parse {html} {
	if {![regexp -- $google::convert_regexp $html -> result]} {
		error "Parse error or no result"
	}
	set result [htmlparse::mapEscapes $result]
	# change <sup>num</sup> to ^num (exponent)
	set result [regsub -all -- {<sup>(.*?)</sup>} $result {^\1}]
	# strip rest of html code
	return [regsub -all -- {<.*?>} $result ""]
}

# Output for results from api query
proc google::output {server chan url title content} {
	regsub -all -- {(?:<b>|</b>)} $title "\002" title
	regsub -all -- {<.*?>} $title "" title
	set output "$title @ $url"
	putchan $server $chan "[htmlparse::mapEscapes $output]"
}

# Query api
proc google::api_handler {server chan argv url {num {}}} {
	if {[string length $argv] == 0} {
		putchan $server $chan "Error: Please supply search terms."
		return
	}
	set query [http::formatQuery v "1.0" q $argv safe off]
	set headers [list Referer $google::api_referer]
	if {$num == ""} {
		set num 4
	}

	http::config -useragent $google::useragent_api
	set token [http::geturl ${url}?${query} -headers $headers -method GET -command "google::api_callback $server $chan $num"]
}

proc google::api_callback {server chan num token} {
	set data [http::data $token]
	set ncode [http::ncode $token]
	http::cleanup $token

	# debug
	#set fid [open "g-debug.txt" w]
	#fconfigure $fid -translation binary -encoding binary
	#puts $fid $data
	#close $fid

	if {$ncode != 200} {
		putchan $server $chan "HTTP query failed: $ncode"
		return
	}

	set data [json::json2dict $data]
	set response [dict get $data responseData]
	set results [dict get $response results]

	if {[llength $results] == 0} {
		putchan $server $chan "No results."
		return
	}

	foreach result $results {
		if {$num != "" && [incr count] > $num} {
			return
		}
		dict with result {
			# $language holds lang in news results, doesn't exist in web results
			if {![info exists language] || $language == "en"} {
				google::output $server $chan $unescapedUrl $title $content
			}
		}
	}
}

# Regular API search
proc google::search {server nick uhost chan argv} {
	if {![str_in_settings_str "google_enabled_channels" $chan]} {
		return
	}

	google::api_handler $server $chan $argv ${google::api_url}web
}

# Regular API search, 1 result
proc google::search1 {server nick uhost chan argv} {
	if {![str_in_settings_str "google_enabled_channels" $chan]} {
		return
	}

	google::api_handler $server $chan $argv ${google::api_url}web 1
}

# News from API
proc google::news {server nick uhost chan argv} {
	if {![str_in_settings_str "google_enabled_channels" $chan]} {
		return
	}

	google::api_handler $server $chan $argv ${google::api_url}news
}

# Images from API
proc google::images {server nick uhost chan argv} {
	if {![str_in_settings_str "google_enabled_channels" $chan]} {
		return
	}

	google::api_handler $server $chan $argv ${google::api_url}images
}

irssi_print "google.tcl loaded"
