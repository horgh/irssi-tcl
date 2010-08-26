set binds(msg_pub) []
set x []

proc emit_msg_pub {server msg nick uhost target} {
	set b "$server $msg $nick $uhost $target"
#	set ::x "$b is cool"
	foreach bind $::binds(msg_pub) {
		$bind $msg $nick $uhost $target
	}
}

proc bind {type proc_name} {
	lappend ::binds($type) $proc_name
}


# eg script
proc repeat {msg nick uhost target} {
	set ::x "Msg: $msg"
}

bind msg_pub repeat
