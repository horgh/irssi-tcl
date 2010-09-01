#
# core Irssi Tcl script
#

# SCRIPT_PATH is relative to ~/.irssi
set SCRIPT_PATH /tcl

set signals(msg_pub) []
set ::tcl_interactive 0

proc bgerror {msg} {
	irssi_print "Error: $msg"
}

# Strip \n, \t, ending whitespace
proc clean {text} {
	regsub -all -- {\n} $text " " text
	regsub -all -- {\t} $text "" text
	set text [string trim $text]
	return $text
}

# Do some cleanup before sending to server
proc putserv {server_tag text} {
	set text [clean $text]
	putserv_raw $server_tag $text
}

proc putchan {server_tag channel text} {
	set text [clean $text]
	putchan_raw $server_tag $channel $text
}

proc emit_msg_pub {server nick uhost target msg} {
	set l [split $msg]

	# first word is keyword
	set keyword [lindex $l 0]
	set rest [join [lrange $l 1 end]]

	foreach bind $::signals(msg_pub) {
		set bind_keyword [lindex $bind 0]
		set bind_proc [lindex $bind 1]
		if {[string match -nocase $bind_keyword $keyword]} {
			# We want to include first word if capturing all
			if {$bind_keyword == "*"} {
				$bind_proc $server $nick $uhost $target $msg
			} else {
				$bind_proc $server $nick $uhost $target $rest
			}
		}
	}
}

# e.g. signal_add msg_pub "!google" google_proc
proc signal_add {type keyword proc_name} {
	lappend ::signals($type) [list $keyword $proc_name]
}

# return 1 if channel is in setting key string (list) $key, 0 otherwise
proc channel_in_settings_str {key channel} {
	set channel_str [settings_get $key]
	set channels [split $channel_str]
	if {[lsearch -exact -nocase $channels $channel] > -1} {
		return 1
	}
	return 0
}

proc load_script {script} {
	source [irssi_dir]${::SCRIPT_PATH}/${script}
}

load_script scripts.conf
