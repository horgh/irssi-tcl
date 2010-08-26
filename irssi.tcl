set binds(msg_pub) []
set ::tcl_interactive 0

proc emit_msg_pub {server msg nick uhost target} {
	foreach bind $::binds(msg_pub) {
		$bind $server $msg $nick $uhost $target
	}
}

proc bind {type proc_name} {
	lappend ::binds($type) $proc_name
}

source scripts/repeat.tcl
source scripts/urltitle.tcl
