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
const char *tcl_result() {
	// TODO it appears we should use Tcl_GetObjResult and work with
	// that rather than this or could possibly lose data
	// see GetStringResult docs
	return Tcl_GetStringResult(interp);
}

/*
 * Concat all strings given in va_list into target with a space between each
 */
void concat_strs(char *target, int len, int num, ...) {
	int i;
	char *str;

	va_list vl;
	va_start(vl, num);

	target[0] = '\0';
	for (i = 0; i < num; i++) {
		str = va_arg(vl, char *);
		strncat(target, str, len - strlen(target) - 1);
		// don't add " " to end
		if (i < num - 1)
			strncat(target, " ", len - strlen(target) - 1);
	}
	
	va_end(vl);
}

/*
 * Irssi command /tcl
 */
static void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item) {
	// /tcl reload
	if (strcmp(data, "reload") == 0) {
		if(tcl_reload_scripts() == 1)
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Scripts reloaded");
		else
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure");
		return;
	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Running /tcl: '%s'", data);
	if (tcl_command(data)) {
		const char *res = tcl_result();
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Result: %s", res);
	} else {
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Error executing /tcl command: %s", data);
	}
}

/*
 * Called when "message public" signal from Irssi
 */
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target) {
	char *cmd = (char *) malloc(MAX_CMD_LEN);
	concat_strs(cmd, MAX_CMD_LEN, 6, "emit_msg_pub", server->tag, msg, nick, address, target);

	printtext(NULL, NULL, MSGLEVEL_CRAP, "%s %s %s %s %s", server-> tag, msg, nick, address, target);
	if (tcl_command(cmd))
		printtext(NULL, NULL, MSGLEVEL_CRAP, "cmd: %s", cmd);
	else
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Error executing tcl command: %s", cmd);
	free(cmd);
	// TODO do we really need to check result here? Seems not.
	//const char *res = tcl_result();
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
	if (Tcl_EvalFile(interp, "/home/will/code/irssi_tcl/binds.tcl") == TCL_OK)
		return 1;
	return -1;
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

	if(!tcl_reload_scripts()) {
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
