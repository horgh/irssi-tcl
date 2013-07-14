// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * signal callback functions.
 */

#include <tcl.h>
#include "irssi_includes.h"
#include "tcl_core.h"

#include "signals.h"

/*
 * Ugly hack to check Tcl events!
 * TODO: Look into hooking into as glib event source
 */
void
time_change(void) {
	int events = 1;
	// XXX: hmm... in some cases at least Tcl_DoOneEvent() returns
	//      how many events it executed, so while() we executed
	//      one does not seem quite right...
	while (events > 0) {
		events = Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
	}
}

/*
	This is called when a user hits enter with a line in a window
*/
void
send_text(char* line, SERVER_REC* server, WI_ITEM_REC* item) {
	int result;
	// Have to do this check as window_item_get_target() is invalid if so
	if (item != NULL) {
		const char* target = window_item_get_target(item);
		result = execute(4, "emit_send_text", server->tag, target, line);
	}	else {
		result = execute(4, "emit_send_text", server->tag, "", line);
	}

	if (result != TCL_OK) {
		const char* const tcl_error = tcl_str_error();
		if (tcl_error) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting send_text"
				" signal: %s", tcl_error);
		} else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting send_text"
				" signal.");
		}
	}

	// I believe this emit_send_text may have been part of a being able
	// to trigger on our own message approach.
	//if (TCL_OK != execute(4, "emit_send_text", server->tag, target, line)) {
}

/*
 * Called when "message public" signal from Irssi
 */
void
msg_pub(SERVER_REC* server, char* msg, const char* nick,
	const char* address, const char* target)
{
	if (TCL_OK != execute(6, "emit_msg_pub", server->tag, nick, address,
		target, msg))
	{
		const char* const tcl_error = tcl_str_error();
		if (tcl_error) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub"
				" signal: %s", tcl_error);
		} else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub"
				" signal.");
		}
	}
	// We don't want to print this any more. Let the Tcl module do it
	//signal_stop();
}

/*
	Triggers on our own public messages
*/
void
msg_own_pub(SERVER_REC* server_rec, char* msg, char* target) {
	if (TCL_OK != execute(6, "emit_msg_pub", server_rec->tag,
		server_rec->nick, "", target, msg))
	{
		const char* const tcl_error = tcl_str_error();
		if (tcl_error) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub"
				" (in server_sendmsg) signal: %s", tcl_error);
		} else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub"
				" (in server_sendmsg) signal.");
		}
	}
}

/*
 * Triggers on message sent by self, but not those sent by message own_public
 * (so that we can work with pub triggers that we trigger ourself)
 * NOTE: This includes msgs to channels (type = 0), and nicks (type = 1)
 */
void
server_sendmsg(SERVER_REC* server, char* target, char* msg, int type) {
	(void) server;
	(void) target;
	(void) msg;
	(void) type;
	// public msg

	// Used to trigger on own messages from here
	/*
	if (type == 0) {
		if (TCL_OK != execute(6, "emit_msg_pub", server->tag, server->nick, "",
			target, msg)) {
			const char* const tcl_error = tcl_str_error();
			if (tcl_error) {
				printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub"
					" (in server_sendmsg) signal: %s", tcl_error);
			} else {
				printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub"
					" (in server_sendmsg) signal.");
			}
		}
	// private msg
	} else {
		// TODO emit msg to trigger on PM
	}
	*/
}
