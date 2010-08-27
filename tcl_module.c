/*
 * 26/08/2010
 * by Will Storey
 *
 * TODO:
 *  - fix so we aren't using the hack "expando timer" signal!
 */

#include <tcl.h>
#include "tcl_module.h"
#include "debug.h"

static Tcl_Interp *interp;

typedef struct {
	const char *type;
	const void *func;
} Signal;

typedef struct {
	const char *cmd;
	const void *func;
} TclCmd;

static const Signal SignalTable[] = {
	{"message public", msg_pub},
	{"expando timer", time_change},
	{NULL, NULL}
};

static const TclCmd TclCmdTable[] = {
	{"putserv_raw", putserv_raw},
	{"putchan_raw", putchan_raw},
	{"irssi_print", irssi_print},
	{"settings_get", settings_get},
	{"settings_add_str", settings_add_str_tcl},
	{"irssi_dir", irssi_dir},
	{NULL, NULL}
};

/*
 * Irssi /commands
 */

void init_commands() {
	command_bind("tcl", NULL, (SIGNAL_FUNC) cmd_tcl);
}

void deinit_commands() {
	command_unbind("tcl", (SIGNAL_FUNC) cmd_tcl);
}

/*
 * /tcl
 * /tcl reload
 */
static void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item) {
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
	if (tcl_command(data)) {
		const char *res = tcl_str_result();
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Result: %s", res);
	} else {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error executing /tcl command: %s", data);
	}
}

/*
 * Signals
 */

void init_signals() {
	int i;
	for (i = 0; SignalTable[i].type != NULL; i++)
		signal_add_first(SignalTable[i].type, (SIGNAL_FUNC) SignalTable[i].func);
}

void deinit_signals() {
	int i;
	for (i = 0; SignalTable[i].type != NULL; i++)
		signal_remove(SignalTable[i].type, (SIGNAL_FUNC) SignalTable[i].func);
}

/*
 * Called when "message public" signal from Irssi
 */
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target) {
	if (TCL_OK != execute(6, "emit_msg_pub", server->tag, nick, address, target, msg)) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Error emitting msg_pub signal");
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

/*
 * Tcl interpreter commands
 */

void tcl_register_commands() {
	int i;
	for (i = 0; TclCmdTable[i].cmd != NULL; i++)
		Tcl_CreateObjCommand(interp, TclCmdTable[i].cmd, TclCmdTable[i].func, NULL, NULL);
}

/*
 * Return string corresponding to full path to ~/.irssi
 */
int irssi_dir(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 1) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putserv_raw server_tag text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_DString dsPtr;
	Tcl_DStringInit(&dsPtr);
	irssi_dir_ds(&dsPtr, "");
	Tcl_DStringResult(interp, &dsPtr);
	Tcl_DStringFree(&dsPtr);
	return TCL_OK;
}

/*
 * putserv_raw tcl interp command
 * TODO: Any output from this command will not be seen on Irssi side
 */
int putserv_raw(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 3) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putserv_raw server_tag text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	// objv[0] has "putserv_raw"
	char *server_tag = Tcl_GetString(objv[1]);
	char *text = Tcl_GetString(objv[2]);
	SERVER_REC *server = server_find_tag(server_tag);

	irc_send_cmd((IRC_SERVER_REC *) server, text);

	// TODO need this; but must not listen for own
	//signal_emit("server incoming", 2, server, text);

	return TCL_OK;
}

/*
 * putchan_raw <server_tag> <#chan> <text>
 * Use this instead of putserv so that can see own message
 *
 * "raw" because putchan in Tcl will do some string fixing on text
 */
 int putchan_raw(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 4) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putchan_raw server_tag channel text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	char *server_tag = Tcl_GetString(objv[1]);
	char *chan = Tcl_GetString(objv[2]);
	char *text = Tcl_GetString(objv[3]);
	SERVER_REC *server = server_find_tag(server_tag);
	Tcl_Obj *send_str = Tcl_ObjPrintf("PRIVMSG %s :%s", chan, text);

	irc_send_cmd((IRC_SERVER_REC *) server, Tcl_GetString(send_str));
	signal_emit("message own_public", 3, server, text, chan);
	return TCL_OK;
 }

/*
 * Print string to Irssi from Tcl
 */
int irssi_print(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 2) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"irssi_print string\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	char *str = Tcl_GetString(objv[1]);
	printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: %s", str);
	return TCL_OK;
}

/*
 * settings_get Tcl command to get Irssi settings
 */
int settings_get(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 2) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"settings_get settings_key\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	char *key = Tcl_GetString(objv[1]);
	const char *value = settings_get_str(key);
	if (value == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("error: setting key not found");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	Tcl_Obj *str = Tcl_NewStringObj(value, strlen(value));
	Tcl_SetObjResult(interp, str);
	return TCL_OK;
}

/*
 * Add string setting from Tcl
 *
 * named this way as conflicts with macro...
 */
int settings_add_str_tcl(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 3) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"settings_add_str key default\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	char *key = Tcl_GetString(objv[1]);
	char *def = Tcl_GetString(objv[2]);
	settings_add_str("tcl", key, def);
	return TCL_OK;
}

/*
 * Foundation functions
 */

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
	if (Tcl_Eval(interp, cmd) == TCL_OK)
		return -1;
	return 1;
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
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Script initialisation error: %s", tcl_str_error());
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
