#
# core Irssi Tcl script
#

# SCRIPT_PATH is relative to ~/.irssi
set SCRIPT_PATH /tcl

set signals(msg_pub) []
set signals(send_text) []
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

proc emit_send_text {server target line} {
	# The keyword part of signals list has no meaning for send_text
	foreach bind $::signals(send_text) {
		set bind_proc [lindex $bind 1]
		$bind_proc $server $target $line
	}
}

# e.g. signal_add msg_pub "!google" google_proc
proc signal_add {type keyword proc_name} {
	lappend ::signals($type) [list $keyword $proc_name]
}

# return 1 if str is in setting key string (list) $key, 0 otherwise
proc str_in_settings_str {key str} {
	set setting_str [settings_get_str $key]
	set strs [split $setting_str]
	if {[lsearch -exact -nocase $strs $str] > -1} {
		return 1
	}
	return 0
}

proc load_script {script} {
	source [irssi_dir]${::SCRIPT_PATH}/${script}
}

proc load_script_absolute {script} {
	source $script
}

load_script scripts.conf
