// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * Capture signals sent by Irssi and send them to Tcl
 */

#include <tcl.h>
#include "module.h"
#include "irssi_includes.h"

#include "signals.h"

void
init_signals(void) {
	signal_add_first("expando timer", (SIGNAL_FUNC) time_change);
	signal_add_first("send_text", (SIGNAL_FUNC) send_text);

	signal_add_last("message public", (SIGNAL_FUNC) msg_pub);
	signal_add_last("message own_public", (SIGNAL_FUNC) msg_own_pub);
	signal_add_last("server sendmsg", server_sendmsg);
}

void
deinit_signals(void) {
	signal_remove("expando timer", (SIGNAL_FUNC) time_change);
	signal_remove("send_text", (SIGNAL_FUNC) send_text);

	signal_remove("message public", (SIGNAL_FUNC) msg_pub);
	signal_remove("message own_public", (SIGNAL_FUNC) msg_own_pub);
	signal_remove("server sendmsg", (SIGNAL_FUNC) server_sendmsg);
}

/*
 * Called when "message public" signal from Irssi
 */
void
msg_pub(SERVER_REC* server, char* msg, const char* nick,
	const char* address, const char* target)
{
	if (TCL_OK != execute(6, "emit_msg_pub", server->tag, nick, address, target, msg)) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub signal: %s", tcl_str_error());
	}
	// We don't want to print this any more. Let the Tcl module do it
	//signal_stop();
}

/*
	Triggers on our own public messages
*/
void
msg_own_pub(SERVER_REC* server_rec, char* msg, char* target) {
	if (TCL_OK != execute(6, "emit_msg_pub", server_rec->tag, server_rec->nick, "", target, msg)) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub (in server_sendmsg) signal: %s", tcl_str_error());
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
		const char *target = window_item_get_target(item);
		result = execute(4, "emit_send_text", server->tag, target, line);
	}	else {
		result = execute(4, "emit_send_text", server->tag, "", line);
	}

	if (result != TCL_OK)
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting send_text signal: %s", tcl_str_error());
	//if (TCL_OK != execute(4, "emit_send_text", server->tag, target, line)) {
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
		if (TCL_OK != execute(6, "emit_msg_pub", server->tag, server->nick, "", target, msg)) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub (in server_sendmsg) signal: %s", tcl_str_error());
		}
	// private msg
	} else {
		// TODO emit msg to trigger on PM
	}
	*/
}

/*
 * Ugly hack to check Tcl events!
 * TODO: Look into hooking into as glib event source
 */
void
time_change(void) {
	int events = 1;
	while (events > 0)
		events = Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
}
