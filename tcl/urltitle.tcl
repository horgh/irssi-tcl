#
# Fetch title of URLs in channels
#
# /set urltitle_enabled_channels #channel1 #channel2 ..
# to enable
#

package require http
package require tls
package require htmlparse
package require idna

namespace eval urltitle {
	#variable useragent "Lynx/2.8.7rel.1 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/0.9.8n"
	variable useragent "Tcl http client package 2.7.5"
	variable max_bytes 32768

	settings_add_str "urltitle_enabled_channels" ""

	signal_add msg_pub "*" urltitle::urltitle

	::http::register https 443 ::tls::socket
}

proc urltitle::urltitle {server nick uhost target msg} {
	if {![channel_in_settings_str urltitle_enabled_channels $target]} {
		return
	}
	set full_url []
	if {[regexp -nocase -- {(https?://\S+)} $msg -> url]} {
		set full_url $url
	} elseif {[regexp -nocase -- {(www\.\S+)} $msg -> url]} {
		set full_url "http://${url}"
	}

	if {$full_url == ""} {
		return
	}

	if {![regexp -- {(https?://)([^/]*)/?(.*)} $full_url -> prefix domain rest]} {
		error "urltitle parse problem: $full_url"
	}
	set domain [idna::domain_toascii $domain]
	#set rest [http::formatQuery $rest]

	# from http-title.tcl by Pixelz. Avoids urls that will be treated as
	# a flag
	if {[string index $domain 0] eq "-"} {
		return
	}

	set url "${prefix}${domain}/${rest}"
	urltitle::geturl $url $server $target
}

proc urltitle::extract_title {data} {
	if {[regexp -nocase -- {<title>(.*?)</title>} $data -> title]} {
		set title [regsub -all -- {\s+} $title " "]
		return [htmlparse::mapEscapes $title]
	}
	return ""
}

proc urltitle::geturl {url server target} {
	http::config -useragent $urltitle::useragent
	set token [http::geturl $url -blocksize $urltitle::max_bytes -timeout 10000 \
		-progress urltitle::http_progress -command "urltitle::http_done $server $target"]
}

# stop after max_bytes
proc urltitle::http_progress {token total current} {
	if {$current >= $urltitle::max_bytes} {
		http::reset $token
	}
}

proc urltitle::http_done {server target token} {
	set data [http::data $token]
	set code [http::ncode $token]
	set meta [http::meta $token]
	set charset [urltitle::get_charset $token]
	http::cleanup $token

	# Follow redirects for some 30* codes
	if {[regexp -- {30[01237]} $code]} {
		urltitle::geturl [dict get $meta Location] $server $target
	} else {
		set data [encoding convertfrom $charset $data]
		set title [extract_title $data]
		if {$title != ""} {
			putchan $server $target "\002[string trim $title]"
		}
	}
}

proc urltitle::get_charset {token} {
	upvar #0 $token state
	if {[info exists state(charset)]} {
		set charset $state(charset)
	} else {
		set charset iso8859-1
	}
	# needed as some charsets from state(charset) are invalid mapping
	# to tcl charsets. e.g. iso-8859-1 must be changed to iso-8859-1
	regsub -- {iso-} $charset iso charset
	return $charset
}

irssi_print "urltitle.tcl loaded"
