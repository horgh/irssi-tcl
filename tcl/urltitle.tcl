#
# Fetch title of URLs in channels
#
# /set urltitle_enabled_channels #channel1 #channel2 ..
# to enable in those channels
#
# /set urltitle_ignored_nicks nick1 nick2 nick2 ..
# to not fetch titles of urls by these nicks
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
	settings_add_str "urltitle_ignored_nicks" ""

	signal_add msg_pub "*" urltitle::urltitle

	::http::register https 443 ::tls::socket

	variable debug 0
}

proc urltitle::urltitle {server nick uhost chan msg} {
	if {![str_in_settings_str urltitle_enabled_channels $chan]} {
		return
	}

	if {[str_in_settings_str urltitle_ignored_nicks $nick]} {
		return
	}

	set url [urltitle::recognise_url $msg]
	if {$url == ""} {
		return
	}

	urltitle::geturl $url $server $chan
}

# Breaks an absolute URL into 3 pieces:
# prefix/protocol: e.g. http://, https//
# domain: e.g. everything up to the first /, if it exists
# rest: everything after the first /, if exists
proc urltitle::split_url {absolute_url} {
	if {![regexp -- {(https?://)([^/]*)/?(.*)} $absolute_url -> prefix domain rest]} {
		error "urltitle error: parse problem: $absolute_url"
	}
	set domain [idna::domain_toascii $domain]

	# from http-title.tcl by Pixelz. Avoids urls that will be treated as
	# a flag
	if {[string index $domain 0] eq "-"} {
		error "urltitle error: Invalid URL: domain looks like a flag"
	}
	return [list $prefix $domain $rest]
}

# Attempt to recognise potential_url as an actual url in form of http[s]://...
# Returns blank if unsuccessful
proc urltitle::recognise_url {potential_url} {
	set full_url []
	if {[regexp -nocase -- {(https?://\S+)} $potential_url -> url]} {
		set full_url $url
	} elseif {[regexp -nocase -- {(www\.\S+)} $potential_url -> url]} {
		set full_url "http://${url}"
	}

	if {$full_url == ""} {
		return ""
	}

	lassign [urltitle::split_url $full_url] prefix domain rest

	return "${prefix}${domain}/${rest}"
}

proc urltitle::extract_title {data} {
	if {[regexp -nocase -- {<title>(.*?)</title>} $data -> title]} {
		set title [regsub -all -- {\s+} $title " "]
		return [htmlparse::mapEscapes $title]
	}
	return ""
}

proc urltitle::geturl {url server chan} {
	if {$urltitle::debug} {
		irssi_print "urltitle debug: Trying to get URL: $url"
	}
	http::config -useragent $urltitle::useragent
	set token [http::geturl $url -blocksize $urltitle::max_bytes -timeout 10000 \
		-progress urltitle::http_progress -command "urltitle::http_done $server $chan $url"]
}

# stop after max_bytes
proc urltitle::http_progress {token total current} {
	if {$current >= $urltitle::max_bytes} {
		http::reset $token
	}
}

proc urltitle::http_done {server chan url token} {
	set data [http::data $token]
	set code [http::ncode $token]
	set meta [http::meta $token]
	if {$urltitle::debug} {
		irssi_print "data ${data}"
		irssi_print "code ${code}"
		irssi_print "meta ${meta}"
	}
	set charset [urltitle::get_charset $token]
	http::cleanup $token

	# Follow redirects for some 30* codes
	if {[regexp -- {30[01237]} $code]} {
		# Location may not be an absolute URL
		set new_url [urltitle::make_absolute_url $url [dict get $meta Location]]
		urltitle::geturl $new_url $server $chan
	} else {
		set data [encoding convertfrom $charset $data]
		set title [extract_title $data]
		if {$title != ""} {
			putchan $server $chan "\002[string trim $title]"
		}
	}
}

# Ensure we return an absolute URL
# new_target is the Location given by a redirect. This may be an absolute
# url, or it may be relative
# If it's relative, use old_url
proc urltitle::make_absolute_url {old_url new_target} {
	# First check if we've been given an absolute URL
	set absolute_url [urltitle::recognise_url $new_target]
	if {$absolute_url != ""} {
		return $absolute_url
	}

	# Otherwise it must be a relative URL
	lassign [urltitle::split_url $old_url] prefix domain rest

	# Take everything up to the last / from rest
	if {[regexp -- {(\S+)/} $rest -> rest_prefix]} {
		set new_url "${prefix}${domain}/${rest_prefix}/${new_target}"

	# Otherwise there was no / in rest, so at top level
	} else {
		set new_url "${prefix}${domain}/${new_target}"
	}

	if {$urltitle::debug} {
		irssi_print "urltitle debug: make_absolute_url: prefix: $prefix domain $domain rest $rest old_url $old_url new_url $new_url"
	}

	return $new_url
}

proc urltitle::get_charset {token} {
	upvar #0 $token state
	if {[info exists state(charset)]} {
		set charset $state(charset)
	} else {
		set charset iso8859-1
	}
	# needed as some charsets from state(charset) are invalid mapping
	# to tcl charsets.

	# iso-8859-1 must be changed to iso8859-1
	regsub -- {iso-} $charset iso charset
	# shift_jis -> shiftjis
	regsub -- {shift_} $charset shift charset
	return $charset
}

irssi_print "urltitle.tcl loaded"
