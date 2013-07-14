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

/*
 * /tcl
 * /tcl reload
 */
void
cmd_tcl(const char* data, void* server, WI_ITEM_REC* item) {
	// XXX: why do we pass these if we don't need them?
	(void) server;
	(void) item;

	// /tcl reload
	if (strcmp(data, "reload") == 0) {
		if (tcl_reload_scripts() == TCL_OK) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Scripts reloaded");
		} else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure: %s",
				tcl_str_error());
		}
		return;
	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Running /tcl: '%s'", data);
	if (tcl_command(data) == TCL_OK) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Result: %s", tcl_str_result());
	} else {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error executing /tcl"
			" command '%s': %s", data, tcl_str_error());
	}
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
