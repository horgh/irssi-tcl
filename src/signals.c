/*
 * Capture signals sent by Irssi and send them to Tcl
 */

#include <tcl.h>
#include "irssi_includes.h"
#include "signals.h"
#include "module.h"

typedef struct {
	const char *type;
	const void *func;
} Signal;

// added with signal_add_first
static const Signal SignalTableFirst[] = {
	{"message public", msg_pub},
	{"expando timer", time_change},
	{NULL, NULL}
};

// added with signal_add_last
static const Signal SignalTableLast[] = {
	{"server sendmsg", server_sendmsg},
	{NULL, NULL}
};

void init_signals() {
	int i;
	for (i = 0; SignalTableFirst[i].type != NULL; i++)
		signal_add_first(SignalTableFirst[i].type, (SIGNAL_FUNC) SignalTableFirst[i].func);
	for (i = 0; SignalTableLast[i].type != NULL; i++)
		signal_add_last(SignalTableLast[i].type, (SIGNAL_FUNC) SignalTableLast[i].func);
}

void deinit_signals() {
	int i;
	for (i = 0; SignalTableFirst[i].type != NULL; i++)
		signal_remove(SignalTableFirst[i].type, (SIGNAL_FUNC) SignalTableFirst[i].func);
	for (i = 0; SignalTableLast[i].type != NULL; i++)
		signal_remove(SignalTableLast[i].type, (SIGNAL_FUNC) SignalTableLast[i].func);
}

/*
 * Called when "message public" signal from Irssi
 */
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target) {
	if (TCL_OK != execute(6, "emit_msg_pub", server->tag, nick, address, target, msg)) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub signal: %s", tcl_str_error());
	}
}

/*
 * Triggers on message sent by self, but not those sent by message own_public
 * (so that we can work with pub triggers that we trigger ourself)
 * NOTE: This includes msgs to channels (type = 0), and nicks (type = 1)
 */
void server_sendmsg(SERVER_REC *server, char *target, char *msg, int type) {
	// public msg
	if (type == 0) {
		if (TCL_OK != execute(6, "emit_msg_pub", server->tag, "", "", target, msg)) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub (in server_sendmsg) signal: %s", tcl_str_error());
		}
	// private msg
	} else {
		// TODO emit msg to trigger on PM
	}
}

/*
 * Ugly hack to check Tcl events!
 * TODO: Look into hooking into as glib event source
 */
void time_change() {
	int events = 1;
	while (events > 0)
		events = Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
}

