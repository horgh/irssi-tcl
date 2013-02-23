// vim: tabstop=2:shiftwidth=2:noexpandtab
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
int
irssi_dir(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj *const objv[])
{
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
 *	Stops the current signal
 */
int
sig_stop(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[])
{
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
 * all command parameters should be using unicode (internal) encoding.
 *
 * TODO: Any output from this command will not be seen on Irssi side
 */
int
putserv_raw(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[])
{
	if (objc != 3) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putserv_raw server_tag text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const server_tag = objv[1];
	Tcl_Obj* const text = objv[2];

	// find the Irssi server with the given tag.
	SERVER_REC *server = server_find_tag(Tcl_GetString(server_tag));
	if (server == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("server with tag '%s' not found",
			Tcl_GetString(server_tag));
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	irc_send_cmd((IRC_SERVER_REC *) server, Tcl_GetString(text));
	return TCL_OK;
}

/*
 * putchan_raw <server_tag> <#chan> <text>
 * Use this instead of putserv so that can see own message
 *
 * "raw" because putchan in Tcl will do some string fixing on text
 *
 * all command parameters should be using unicode (internal) encoding.
 */
int
putchan_raw(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[])
{
	if (objc != 4) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putchan_raw"
			" server_tag channel text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const server_tag = objv[1];
	Tcl_Obj* const target = objv[2];
	Tcl_Obj* const msg = objv[3];

	// find the server in Irssi.
	SERVER_REC *server_rec = server_find_tag(Tcl_GetString(server_tag));
	if (server_rec == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("server with tag '%s' not found",
			Tcl_GetString(server_tag));
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	// find the channel on this server in Irssi.
	CHANNEL_REC* channel_rec = channel_find(server_rec, Tcl_GetString(target));
	if (channel_rec == NULL) {
		Tcl_Obj* str = Tcl_ObjPrintf("channel '%s' not found on server '%s'",
			Tcl_GetString(target), Tcl_GetString(server_tag));
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	// create the full command string to send to the IRC server.
	// PRIVMSG <target> :<msg>

	// this is how we used to create the command but I am concerned it
	// is not dealing with encoding correctly.
	//Tcl_Obj *send_str = Tcl_ObjPrintf("PRIVMSG %s :%s", target, msg);

	// try to be more careful with how we build the string.
	Tcl_Obj *send_str = Tcl_NewStringObj("PRIVMSG ", strlen("PRIVMSG "));
	if (!send_str) {
		return TCL_ERROR;
	}
	Tcl_AppendObjToObj(send_str, target);
	Tcl_AppendToObj(send_str, " :", strlen(" :"));
	Tcl_AppendObjToObj(send_str, msg);

	// send the command to the server.
	irc_send_cmd((IRC_SERVER_REC *) server_rec, Tcl_GetString(send_str));

	// write the message to Irssi so we see it ourselves.
	print_message_public(server_rec, channel_rec, Tcl_GetString(target),
		server_rec->nick, NULL, Tcl_GetString(msg));

	//signal_emit("message own_public", 3, server, text, chan);
	return TCL_OK;
}

/*
 * Emit a message public event
 * Does not actually send anything to the server, only to the client
 */
int
emit_message_public(ClientData clientData, Tcl_Interp* interp,
	int objc, Tcl_Obj* const objv[])
{
	// emit_message_public <server> <channel> <nick> <address> <text>
	if (objc != 6) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"emit_message_public server channel nick address text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const server_tag = objv[1];
	Tcl_Obj* const chan = objv[2];
	Tcl_Obj* const nick = objv[3];
	Tcl_Obj* const addr = objv[4];
	Tcl_Obj* const text = objv[5];

	// find the Irssi server by tag.
	SERVER_REC *server = server_find_tag(Tcl_GetString(server_tag));
	if (server == NULL) {
		Tcl_Obj *str = Tcl_ObjPrintf("server with tag '%s' not found",
			Tcl_GetString(server_tag));
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	signal_emit("message public", 5, server,
		Tcl_GetString(text), Tcl_GetString(nick),
		Tcl_GetString(addr), Tcl_GetString(chan));
	return TCL_OK;
}

/*
 * Similar to emit_message_public() in that the public message will be
 * printed to the Irssi screen as though it is a public message, but this
 * one does not cause an Irssi signal to be sent.
 */
int
print_message_public_tcl(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[])
{
	// print_message_public <server> <channel> <nick> <address> <text>
	if (objc != 6) {
		Tcl_Obj* str = Tcl_ObjPrintf("wrong # args: should be \"print_message_public server channel nick address text \"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const server_tag = objv[1];
	// channel = target
	Tcl_Obj* const target = objv[2];
	Tcl_Obj* const nick = objv[3];
	Tcl_Obj* const address = objv[4];
	Tcl_Obj* const msg = objv[5];

	// find the Irssi server by tag.
	SERVER_REC* server_rec = server_find_tag(Tcl_GetString(server_tag));
	if (server_rec == NULL) {
		Tcl_Obj* str = Tcl_ObjPrintf("server with tag '%s' not found",
			Tcl_GetString(server_tag));
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	// find the channel on the server.
	CHANNEL_REC* channel_rec = channel_find(server_rec,
		Tcl_GetString(target));
	if (channel_rec == NULL) {
		Tcl_Obj* str = Tcl_ObjPrintf("channel '%s' not found on server '%s'",
			Tcl_GetString(target), Tcl_GetString(server_tag));
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	print_message_public(server_rec, channel_rec,
		Tcl_GetString(target), Tcl_GetString(nick),
		Tcl_GetString(address), Tcl_GetString(msg));
	return TCL_OK;
}

/*
 * Print string to Irssi from Tcl
 */
int
irssi_print(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[])
{
	if (objc != 2) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"irssi_print string\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const str = objv[1];

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: %s", Tcl_GetString(str));
	return TCL_OK;
}

/*
 * settings_get_str Tcl command to get Irssi settings
 */
int
settings_get_str_tcl(ClientData clientData, Tcl_Interp* interp,
	int objc, Tcl_Obj* const objv[])
{
	if (objc != 2) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"settings_get_str settings_key\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const key = objv[1];

	// find the setting value.
	const char *value = settings_get_str(Tcl_GetString(key));
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
int
settings_add_str_tcl(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[])
{
	if (objc != 3) {
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"settings_add_str key default\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}
	Tcl_Obj* const key = objv[1];
	Tcl_Obj* const def = objv[2];

	settings_add_str("tcl", Tcl_GetString(key), Tcl_GetString(def));
	return TCL_OK;
}
