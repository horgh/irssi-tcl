/*
 * Commands available in the Tcl interpreter
 */

#include <tcl.h>
#include "irssi_includes.h"
#include "module.h"
#include "tcl_commands.h"

/*
 * Return string corresponding to full path to ~/.irssi
 */
int irssi_dir(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 1) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"irssi_dir\"");
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
	Stops the current signal
*/
int sig_stop(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 1) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"signal_stop\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	signal_stop();
	return TCL_OK;
}

/*
 * putserv_raw tcl interp command
 *
 * TODO: Any output from this command will not be seen on Irssi side
 */
int putserv_raw(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 3) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putserv_raw server_tag text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	char *server_tag = Tcl_GetString(objv[1]);
	char *text = Tcl_GetString(objv[2]);
	SERVER_REC *server = server_find_tag(server_tag);
	if (server == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("server with tag '%s' not found", server_tag);
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	irc_send_cmd((IRC_SERVER_REC *) server, text);
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
	if (server == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("server with tag '%s' not found", server_tag);
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	Tcl_Obj *send_str = Tcl_ObjPrintf("PRIVMSG %s :%s", chan, text);

	irc_send_cmd((IRC_SERVER_REC *) server, Tcl_GetString(send_str));
	signal_emit("message own_public", 3, server, text, chan);
	return TCL_OK;
}

/*
	Emit a message public event
	Does not actually send anything to the server, only to the client
*/
int
emit_message_public(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	// emit_message_public <server> <channel> <nick> <address> <text>
	if (objc != 6) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"emit_message_public server channel nick address text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	char *server_tag = Tcl_GetString(objv[1]);
	char *chan = Tcl_GetString(objv[2]);
	char *nick = Tcl_GetString(objv[3]);
	char *addr = Tcl_GetString(objv[4]);
	char *text = Tcl_GetString(objv[5]);
	SERVER_REC *server = server_find_tag(server_tag);
	if (server == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("server with tag '%s' not found", server_tag);
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	signal_emit("message public", 5, server, text, nick, addr, chan);
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
 * settings_get_str Tcl command to get Irssi settings
 */
int settings_get_str_tcl(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 2) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"settings_get_str settings_key\"");
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
