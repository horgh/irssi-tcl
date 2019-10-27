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
cmd_tcl(
		char const * data,
		const void * unused2,
		const void * unused3,
		void const * unused4,
		void const * unused5,
		void const * unused6
) {
	(void) unused2;
	(void) unused3;
	(void) unused4;
	(void) unused5;
	(void) unused6;

	// /tcl reload
	if (strcmp(data, "reload") == 0) {
		if (tcl_reload_scripts()) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Scripts reloaded");
		} else {
			const char* const tcl_error = tcl_str_error();
			if (tcl_error) {
				printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure: %s",
					tcl_error);
			} else {
				printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure.");
			}
		}
		return;
	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Running /tcl: '%s'", data);
	if (tcl_command(data) == TCL_OK) {
		const char* const tcl_result = tcl_str_result();
		if (tcl_result) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Result: %s", tcl_result);
		} else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Result:");
		}
	} else {
		const char* const tcl_error = tcl_str_error();
		if (tcl_error) {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error executing /tcl"
				" command '%s': %s", data, tcl_error);
		} else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error executing /tcl"
				" command '%s'.", data);
		}
	}
}
