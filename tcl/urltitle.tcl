#
# Fetch title of URLs in channels
#
# /set urltitle_enabled_channels #channel1 #channel2 ..
# to enable
#

package require http
package require tls

namespace eval urltitle {
	variable useragent "Lynx/2.8.7rel.1 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/0.9.8n"
	variable max_bytes 32768

	settings_add_str "urltitle_enabled_channels" ""

	signal_add msg_pub "*" urltitle::urltitle

	::http::register https 443 ::tls::socket
}

proc urltitle::urltitle {server nick uhost target msg} {
	if {![channel_in_settings_str urltitle_enabled_channels $target]} {
		return
	}
	if {[regexp -nocase -- {(https?://\S+)} $msg -> url]} {
		urltitle::geturl $url $server $target
	} elseif {[regexp -nocase -- {(www\.\S+)} $msg -> url]} {
		urltitle::geturl "http://${url}" $server $target
	}
}

proc urltitle::extract_title {data} {
	if {[regexp -nocase -- {<title>(.*?)</title>} $data -> title]} {
		set title [encoding convertfrom identity $title]
		return $title
	}
	return ""
}

proc urltitle::geturl {url server target} {
	http::config -useragent $urltitle::useragent
	set token [http::geturl $url -blocksize $urltitle::max_bytes \
		-progress urltitle::http_progress -command "urltitle::http_done $server $target"]
}

# stop after max_bytes
proc urltitle::http_progress {token total current} {
	if {$current >= 32768} {
		http::reset $token
	}
}

proc urltitle::http_done {server target token} {
	set data [http::data $token]
	set code [http::ncode $token]
	set meta [http::meta $token]
	http::cleanup $token

	# Follow redirects for some 30* codes
	if {[regexp -- {30[01237]} $code]} {
		urltitle::geturl [dict get $meta Location] $server $target
	} else {
		set title [extract_title $data]
		if {$title != ""} {
			putchan $server $target "\002[string trim $title]"
		}
	}
}

irssi_print "urltitle.tcl loaded"
