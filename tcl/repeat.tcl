#
# Example script
# Repeat everything!
#

namespace eval repeat {
	signal_add msg_pub "*" repeat::repeat
}

proc repeat::repeat {server nick uhost target msg} {
	putserv $server "PRIVMSG $target :Repeated: $msg"
}
