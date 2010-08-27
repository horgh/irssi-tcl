# urltitle

package require http

set ::useragent "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.11) Gecko/20100721 Firefox/3.0.6"

signal_add msg_pub "*" urltitle

proc urltitle {server nick uhost target msg} {
	if {[regexp -nocase -- {(http://\S+)} $msg -> url]} {
		geturl $url $server $target
	} elseif {[regexp -nocase -- {(www\.\S+)} $msg -> url]} {
		geturl "http://${url}" $server $target
	}
}

proc extract_title {data} {
	if {[regexp -nocase -- {<title>(.*?)</title>} $data -> title]} {
		set title [encoding convertfrom identity $title]
		return $title
	}
	return ""
}

proc geturl {url server target} {
	puts "Geturl begin with $url"
	http::config -useragent $::useragent
	set token [http::geturl $url -blocksize 1024 -progress http_progress -command "http_done $server $target"]
	#set token [http::geturl $url -blocksize 1024 -progress http_progress]
	#set data [http::data $token]
	#http::cleanup $token
	#if {[regexp -nocase -- {<title>(.*?)</title>} $data -> title]} {
#		set title [encoding convertfrom identity $title]
#		putserv $server "PRIVMSG $target :Title: \002$title"
#	} else {
#		putserv $server "PRIVMSG $target :No title found"
#	}
}

# Stop after 32k
proc http_progress {token total current} {
	if {$current >= 32768} {
		http::reset $token
	}
}

proc http_done {server target token} {
	set data [http::data $token]
	set code [http::ncode $token]
	set meta [http::meta $token]
	http::cleanup $token
	puts "Got code $code"
	puts "Meta $meta"
	puts "Got data $data"

	# Follow redirects
	if {[regexp -- {30[01237]} $code]} {
		puts "Redirecting!!!! location is [dict get $meta Location]"
		geturl [dict get $meta Location] $server $target
	} else {
		set title [extract_title $data]
		if {$title != ""} {
			putserv $server "PRIVMSG $target :\002[string trim $title]"
		}
	}
}
