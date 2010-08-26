#include <tcl.h>
#include "tcl_module.h"

#define MAX_CMD_LEN 1024

static Tcl_Interp *interp;

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
	for (i = 0; i < num; i++) {
		Tcl_DecrRefCount(objv[i]);
	}
	ckfree((char *) objv);

	return result;
}

/*
 * Irssi command /tcl
 */
static void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item) {
	// /tcl reload
	if (strcmp(data, "reload") == 0) {
		if(tcl_reload_scripts() == TCL_OK)
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Scripts reloaded");
		else
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure");
		return;
	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Running /tcl: '%s'", data);
	if (tcl_command(data)) {
		const char *res = tcl_str_result();
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Result: %s", res);
	} else {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Error executing /tcl command: %s", data);
	}
}

/*
 * Called when "message public" signal from Irssi
 */
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target) {
	if (TCL_OK != execute(6, "emit_msg_pub", server->tag, msg, nick, address, target)) {

	} else {

	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "%s %s %s %s %s", server->tag, msg, nick, address, target);
//	if (tcl_command(cmd))
//		printtext(NULL, NULL, MSGLEVEL_CRAP, "cmd: %s", cmd);
//	else
//		printtext(NULL, NULL, MSGLEVEL_CRAP, "Error executing tcl command: %s", cmd);
	// TODO do we really need to check result here? Seems not.
	//const char *res = tcl_str_result();
	//printtext(NULL, NULL, MSGLEVEL_CRAP, "Result: %s", res);
}

/*
 * putserv tcl interp command
 */
int putserv(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 3) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putserv server_tag text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	// will have putserv
	//char *cmd = Tcl_GetStringFromObj(objv[0], NULL);
	char *server_tag = Tcl_GetStringFromObj(objv[1], NULL);
	char *text = Tcl_GetStringFromObj(objv[2], NULL);
	SERVER_REC *server = server_find_tag(server_tag);

	//printf("\ntag: %s cmd: %s text: %s\n", server_tag, cmd, text);
	irc_send_cmdv((IRC_SERVER_REC *) server, text);
	//server_command(text, server, NULL);
	// TODO Does it make sense for this to be here?
	// Causes error ATM
	//signal_continue(2, server, cmd);
	// TODO must not listen for own
	//signal_emit("server incoming", 2, server, text);

	return TCL_OK;
}

/*
 * Add these commands as commands in the Tcl interpreter
 */
int tcl_register_commands() {
	Tcl_CreateObjCommand(interp, "putserv", putserv, NULL, NULL);
	return 1;
}

// TODO deal with path
int tcl_reload_scripts() {
	return Tcl_EvalFile(interp, "/home/will/code/irssi_tcl/binds.tcl");
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

	if (!tcl_register_commands()) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Command setup error");
		return;
	}

	if(tcl_reload_scripts() != TCL_OK) {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Setup error");
		return;
	}

	command_bind("tcl", NULL, (SIGNAL_FUNC) cmd_tcl);

	signal_add("message public", (SIGNAL_FUNC) msg_pub);
}

/*
 * Irssi module unload
 */
void tcl_deinit(void) {
	command_unbind("tcl", (SIGNAL_FUNC) cmd_tcl);

	Tcl_DeleteInterp(interp);
}
