# Repeat everything!

bind msg_pub repeat

proc repeat {server msg nick uhost target} {
	putserv $server "PRIVMSG $target :repeated: $msg"
}
