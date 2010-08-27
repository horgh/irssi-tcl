# Repeat everything!

signal_add msg_pub "*" repeat

proc repeat {server nick uhost target msg} {
	putserv $server "PRIVMSG $target :repeated: $msg"
}
