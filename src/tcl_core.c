// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * Core Tcl functionality
 */

#include <stdbool.h>
#include <string.h>

#include <tcl.h>

#include "tcl_commands.h"
#include "debug.h"
#include "irssi_includes.h"

#include "tcl_core.h"

static Tcl_Interp* interp;

/*
 * register commands available to Tcl interpreter
 */
void
tcl_register_commands(void) {
	if (!interp) {
		return;
	}

	int i = 0;
	for (i = 0; TclCmdTable[i].cmd != NULL; i++) {
		Tcl_CreateObjCommand(interp, TclCmdTable[i].cmd, TclCmdTable[i].func,
			NULL, NULL);
	}
}

//! Setup the Tcl interpreter
/*!
 * @return bool success
 */
bool
tcl_interp_init(void) {
	// first destroy the interpreter if one exists.
	tcl_interp_deinit();

	interp = Tcl_CreateInterp();
	if (interp == NULL) {
		return false;
	}

	// Allow "package require"s to work
	Tcl_Init(interp);
	Tcl_SetServiceMode(TCL_SERVICE_ALL);

	// Encoding. Force utf-8 for now
	Tcl_SetSystemEncoding(interp, "utf-8");

	// register all Tcl commands.
	tcl_register_commands();

	// load the Tcl scripts.
	if (tcl_load_scripts() != TCL_OK) {
		return false;
	}

	return true;
}

//! destroy the current tcl interpreter if it exists.
/*!
 * @return void
 */
void
tcl_interp_deinit(void) {
	if (interp) {
		Tcl_DeleteInterp(interp);
	}
}

/*
 * Execute command cmd in Tcl interpreter.
 */
int
tcl_command(const char* cmd) {
	if (!interp || !cmd || strlen(cmd) == 0) {
		return TCL_ERROR;
	}
	return Tcl_Eval(interp, cmd);
}

/*
 * Get string result from the Tcl interpreter.
 */
const char*
tcl_str_result(void) {
	if (!interp) {
		return NULL;
	}

	Tcl_Obj* obj = Tcl_GetObjResult(interp);
	const char* str = Tcl_GetString(obj);
	return str;
}

/*
 * Retrieve the error string from the tcl interpreter.
 */
const char*
tcl_str_error(void) {
	if (!interp) {
		return NULL;
	}
	const char* result = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	return result;
}

/*
 * Execute the command given with num arguments, num
 * includes the proc name
 *
 * Arguments must be valid C-strings.
 */
int
execute(int num, ...) {
	if (!interp) {
		return TCL_ERROR;
	}
	if (num < 1) {
		return TCL_ERROR;
	}

	int i = 0;
	char* arg = NULL;
	va_list vl;
	va_start(vl, num);

	// create stringobjs and add to objv
	Tcl_Obj** objv = (Tcl_Obj**) ckalloc((unsigned int) num
		* (unsigned int) sizeof(Tcl_Obj*));
	for (i = 0; i < num; i++) {
		arg = va_arg(vl, char*);
		// -1 means take everything up to first NULL.
		objv[i] = Tcl_NewStringObj(arg, -1);
		Tcl_IncrRefCount(objv[i]);
	}
	va_end(vl);
	int result = Tcl_EvalObjv(interp, num, objv, TCL_EVAL_DIRECT);
	// Ensure string objects get freed
	for (i = 0; i < num; i++) {
		Tcl_DecrRefCount(objv[i]);
	}
	ckfree((char*) objv);

	return result;
}

/*
 * Append full path to ~/.irssi and str to given DString
 *
 * Note: str must be a valid c-string
 */
void
irssi_dir_ds(Tcl_DString* dsPtr, const char* str) {
	#ifdef DEBUG_IRSSI_PATH
	const char* irssi_dir = DEBUG_IRSSI_PATH;
	#else
	// full path to ~/.irssi
	const char* irssi_dir = get_irssi_dir();
	#endif

	// -1 means all up to the null.
	Tcl_DStringAppend(dsPtr, irssi_dir, -1);

	// now ~/.irssi/str
	Tcl_DStringAppend(dsPtr, str, -1);
}

//! load the tcl scripts.
/*!
 * @return int TCL_OK if success
 */
int
tcl_load_scripts(void) {
	if (!interp) {
		return TCL_ERROR;
	}

	Tcl_DString dsPtr;
	Tcl_DStringInit(&dsPtr);
	irssi_dir_ds(&dsPtr, "/tcl/irssi.tcl");
	int result = Tcl_EvalFile(interp, Tcl_DStringValue(&dsPtr));
	Tcl_DStringFree(&dsPtr);
	return result;
}

//! reload the tcl scripts
/*!
 * @return bool whether successful
 *
 * we restart the interpreter to do this.
 */
bool
tcl_reload_scripts(void) {
	return tcl_interp_init();
}
