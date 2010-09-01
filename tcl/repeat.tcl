#
# Example script
# Repeat everything!
#
# This might cause infinite loop, I haven't tested in a few revisions.
#

namespace eval repeat {
	signal_add msg_pub "*" repeat::repeat
}

proc repeat::repeat {server nick uhost target msg} {
	putserv $server "PRIVMSG $target :Repeated: $msg"
}
