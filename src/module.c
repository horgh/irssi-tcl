// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * 2010-08-26
 * by Will Storey
 *
 * the core of the irssi module itself.
 */

#include "irssi_includes.h"
#include "module.h"
#include "signals.h"
#include "tcl_core.h"
#include "irssi_commands.h"

#ifdef DEBUG
#include "debug.h"
#endif


// bind Irssi /commands
void
init_commands(void) {
	command_bind("tcl", NULL, (SIGNAL_FUNC) cmd_tcl);
}

// unbind Irssi /commands
void
deinit_commands(void) {
	command_unbind("tcl", (SIGNAL_FUNC) cmd_tcl);
}

//! set up the signals we listen for.
void
init_signals(void) {
	signal_add_first("expando timer", (SIGNAL_FUNC) time_change);
	signal_add_first("send_text", (SIGNAL_FUNC) send_text);

	signal_add_last("message public", (SIGNAL_FUNC) msg_pub);
	signal_add_last("message own_public", (SIGNAL_FUNC) msg_own_pub);
	signal_add_last("server sendmsg", server_sendmsg);
}

//! clear our signals.
void
deinit_signals(void) {
	signal_remove("expando timer", (SIGNAL_FUNC) time_change);
	signal_remove("send_text", (SIGNAL_FUNC) send_text);

	signal_remove("message public", (SIGNAL_FUNC) msg_pub);
	signal_remove("message own_public", (SIGNAL_FUNC) msg_own_pub);
	signal_remove("server sendmsg", (SIGNAL_FUNC) server_sendmsg);
}

/*
 * Irssi module load
 */
void
tcl_init(void) {
	module_register(MODULE_NAME, "core");

	if (!tcl_interp_init()) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Interp init error");
		return;
	}

	init_commands();
	init_signals();
}

/*
 * Irssi module unload
 */
void
tcl_deinit(void) {
	tcl_interp_deinit();
	deinit_commands();
	deinit_signals();
}
