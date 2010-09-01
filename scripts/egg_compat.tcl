#
# Allow (some) eggdrop Tcl scripts to load with minor modifications
#
# Modifications for pub/pubm calls:
#  1. remove hand function argument in bind proc
#  2. add server argument as first function argument in bind proc
#   e.g. proc someproc {nick uhost hand chan argv} { .. }
#     -> proc someproc {server nick uhost chan argv} { .. }
#  3. alter all putserv/puthelp calls to have $server as their first argument
#   e.g. putserv "PRIVMSG $chan :Hello there"
#     -> putserv $server "PRIVMSG $chan :Hello there"
#   NOTE: putserv will not show output in Irssi. It is better to replace putserv
#     with putchan $server $chan "$text" which will (if a public message...)
#  4. possible gotcha of the above: any calls that are not in the procs that
#     are bound directly which use putserv will require an added argument
#   e.g. proc do_something {chan text} {
#          putserv "PRIVMSG $chan :$text"
#        }
#     -> proc do_something {server chan text} {
#          putserv $server "PRIVMSG $chan :text"
#        }
#
#  The script ./utils/eggdrop_convert.tcl can be used for cases 1-3 (hopefully)
#  But does not deal with putserv to putchan
#

# TODO
proc setudef {flag value} {

}

# TODO
proc channel {cmd chan arg} {
	return 1
}

proc bind {type flag trigger fn} {
	if {$type == "pub" || $type == "pubm"} {
		signal_add msg_pub $trigger $fn
	}
}
