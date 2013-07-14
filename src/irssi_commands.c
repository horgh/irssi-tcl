// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * Irssi command functions.
 *
 * These are the functions that we make available to irssi through the
 * module. for example, '/tcl'.
 */

#include "irssi_includes.h"
#include "tcl_core.h"

#include "irssi_commands.h"

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
