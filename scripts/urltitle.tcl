# urltitle

package require http

set useragent "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.1.11) Gecko/20100721 Firefox/3.0.6"

bind msg_pub urltitle

proc urltitle {server msg nick uhost target} {
	if {[regexp -- {(http://\S+)} $msg -> url]} {
		putserv $server "PRIVMSG $target :Matched $url"
		geturl $url $server $target
	} else {
		putserv $server "PRIVMSG $target :No match"
	}
}

proc geturl {url server target} {
	http::config -useragent $::useragent
	#set token [http::geturl $url -blocksize 1024 -progress http_progress -command http_done]
	set token [http::geturl $url -blocksize 1024 -progress http_progress]
	set data [http::data $token]
	http::cleanup $token
	if {[regexp -- {<title>(.*?)</title>} $data -> title]} {
		set title [encoding convertfrom identity $title]
		putserv $server "PRIVMSG $target :Title: \002$title"
		set f [open /home/will/debug.txt w]
		puts $f $title
		close $f
	} else {
		putserv $server "PRIVMSG $target :No title found"
	}
}

# Stop after 32k
proc http_progress {token total current} {
	if {$current >= 32768} {
		http::reset $token
	}
}

proc http_done {token} {
	set data [http::data $token]
	http::cleanup $token
#	if {[regexp -- {<title>(.*?)</title>} $data -> title]} {
#		putserv 
#	}
}
