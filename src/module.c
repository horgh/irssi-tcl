/*
 * 2010-08-26
 * by Will Storey
 */

#include <tcl.h>
#include "irssi_includes.h"
#include "module.h"
#include "signals.h"
#include "tcl_commands.h"

#ifdef DEBUG
#include "debug.h"
#endif

static Tcl_Interp *interp;

// bind Irssi /commands
void init_commands() {
	command_bind("tcl", NULL, (SIGNAL_FUNC) cmd_tcl);
}

// unbind Irssi /commands
void deinit_commands() {
	command_unbind("tcl", (SIGNAL_FUNC) cmd_tcl);
}

/*
 * /tcl
 * /tcl reload
 */
void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item) {
	// /tcl reload
	if (strcmp(data, "reload") == 0) {
		if(tcl_reload_scripts() == TCL_OK)
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Scripts reloaded");
		else {
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure: %s", tcl_str_error());
		}
		return;
	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Running /tcl: '%s'", data);
	if (tcl_command(data) == TCL_OK) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Result: %s", tcl_str_result());
	} else {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error executing /tcl command '%s': %s", data, tcl_str_error());
	}
}

/*
 * register commands available to Tcl interpreter
 */
void tcl_register_commands() {
	int i;
	for (i = 0; TclCmdTable[i].cmd != NULL; i++)
		Tcl_CreateObjCommand(interp, TclCmdTable[i].cmd, TclCmdTable[i].func, NULL, NULL);
}

/*
 * Setup the Tcl interp
 */
int interp_init() {
	interp = Tcl_CreateInterp();
	if (interp == NULL)
		return -1;
	
	// Allow "package require"s to work
	Tcl_Init(interp);
	Tcl_SetServiceMode(TCL_SERVICE_ALL);

	// Encoding. Force utf-8 for now
	Tcl_SetSystemEncoding(interp, "utf-8");

	return 1;
}

/*
 * Execute command cmd in Tcl interp
 */
int tcl_command(const char *cmd) {
	return Tcl_Eval(interp, cmd);
}

/*
 * Get string result in Tcl interp
 */
const char *tcl_str_result() {
	Tcl_Obj *obj = Tcl_GetObjResult(interp);
	const char *str = Tcl_GetString(obj);
	return str;
}

/*
 * Error string
 */
const char *tcl_str_error() {
	const char *result = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	return result;
}

/*
 * Execute the command given with num arguments, num
 * includes the proc name
 *
 * Arguments must be valid C-strings.
 */
int execute(int num, ...) {
	int i;
	char *arg;
	va_list vl;
	va_start(vl, num);

	// create stringobjs and add to objv
	Tcl_Obj **objv = (Tcl_Obj **) ckalloc(num * sizeof(Tcl_Obj *));
	for (i = 0; i < num; i++) {
		arg = va_arg(vl, char *);
		objv[i] = Tcl_NewStringObj(arg, strlen(arg));
		Tcl_IncrRefCount(objv[i]);
	}
	va_end(vl);
	int result = Tcl_EvalObjv(interp, num, objv, TCL_EVAL_DIRECT);
	// Ensure string objects get freed
	for (i = 0; i < num; i++)
		Tcl_DecrRefCount(objv[i]);
	ckfree((char *) objv);

	return result;
}

/*
 * Append full path to ~/.irssi and str to given DString
 *
 * Note: str must be a valid c-string
 */
void irssi_dir_ds (Tcl_DString *dsPtr, char *str) {
	#ifdef DEBUG
	const char *irssi_dir = DEBUG_IRSSI_PATH;
	#else
	// full path to ~/.irssi
	const char *irssi_dir = get_irssi_dir();
	#endif

	Tcl_DStringAppend(dsPtr, irssi_dir, strlen(irssi_dir));

	// now ~/.irssi/str
	Tcl_DStringAppend(dsPtr, str, strlen(str));
}

int tcl_reload_scripts() {
	Tcl_DString dsPtr;
	Tcl_DStringInit(&dsPtr);
	irssi_dir_ds(&dsPtr, "/tcl/irssi.tcl");
	int result = Tcl_EvalFile(interp, Tcl_DStringValue(&dsPtr));
	Tcl_DStringFree(&dsPtr);
	return result;
}

/*
 * Irssi module load
 */
void tcl_init(void) {
	module_register(MODULE_NAME, "core");

	if(!interp_init()) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Interp init error");
		return;
	}

	tcl_register_commands();

	if(tcl_reload_scripts() != TCL_OK) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Script initialisation error: %s (irssi.tcl not found?)", tcl_str_error());
	}

	init_commands();
	init_signals();
}

/*
 * Irssi module unload
 */
void tcl_deinit(void) {
	deinit_commands();
	deinit_signals();
	Tcl_DeleteInterp(interp);
}
