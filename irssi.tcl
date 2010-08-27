set SCRIPT_PATH /home/will/code/irssi_tcl/scripts

set signals(msg_pub) []
set ::tcl_interactive 0
#interp bgerror {} ::bgerror

proc bgerror {msg} {
	irssi_print "Error: $msg"
	#set f [open /home/will/debug.txt w]
	#puts $f "Got error $msg"
	#close $f
}

proc putserv {server_tag text} {
	regsub -all -- {\n} $text " " text
	regsub -all -- {\t} $text "" text
	set text [string trim $text]
	putserv_raw $server_tag $text
}

proc emit_msg_pub {server nick uhost target msg} {
	set l [split $msg]
	set keyword [lindex $l 0]
	set rest [join [lrange $l 1 end]]
	foreach bind $::signals(msg_pub) {
		set bind_keyword [lindex $bind 0]
		set bind_proc [lindex $bind 1]
		if {[string match -nocase $bind_keyword $keyword]} {
			# We want to include first word if capturing all
			if {$bind_keyword == "*"} {
				{*}$bind_proc $server $nick $uhost $target $msg
			} else {
				{*}$bind_proc $server $nick $uhost $target $rest
			}
		}
	}
}

# e.g. signal_add msg_pub "!google" google_proc
proc signal_add {type keyword proc_name} {
	lappend ::signals($type) [list $keyword $proc_name]
}

proc load_script {script} {
	source ${::SCRIPT_PATH}/$script
}

#load_script repeat.tcl
load_script urltitle.tcl
load_script egg_compat.tcl
load_script google.tcl
