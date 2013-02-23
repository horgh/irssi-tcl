// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * 2010-08-26
 * by Will Storey
 *
 * Set up the Tcl interpreter and Irssi module
 * Also provide Irssi commands
 *
 * And some useful functions that don't really fit elsewhere
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
void
tcl_register_commands(void) {
	int i;
	for (i = 0; TclCmdTable[i].cmd != NULL; i++)
		Tcl_CreateObjCommand(interp, TclCmdTable[i].cmd, TclCmdTable[i].func, NULL, NULL);
}

/*
 * Setup the Tcl interp
 */
int
interp_init(void) {
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
int
tcl_command(const char* cmd) {
	return Tcl_Eval(interp, cmd);
}

/*
 * Get string result in Tcl interp
 */
const char*
tcl_str_result(void) {
	Tcl_Obj *obj = Tcl_GetObjResult(interp);
	const char *str = Tcl_GetString(obj);
	return str;
}

/*
 * Error string
 */
const char*
tcl_str_error(void) {
	const char *result = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
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
	int i = 0;
	char *arg = NULL;
	va_list vl;
	va_start(vl, num);

	// create stringobjs and add to objv
	Tcl_Obj **objv = (Tcl_Obj **) ckalloc((unsigned int) num * (unsigned int) sizeof(Tcl_Obj *));
	for (i = 0; i < num; i++) {
		arg = va_arg(vl, char *);
		// -1 means take everything up to first NULL.
		objv[i] = Tcl_NewStringObj(arg, -1);
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
void
irssi_dir_ds(Tcl_DString* dsPtr, const char* str) {
	#ifdef DEBUG
	const char *irssi_dir = DEBUG_IRSSI_PATH;
	#else
	// full path to ~/.irssi
	const char *irssi_dir = get_irssi_dir();
	#endif

	// -1 means all up to the null.
	Tcl_DStringAppend(dsPtr, irssi_dir, -1);

	// now ~/.irssi/str
	Tcl_DStringAppend(dsPtr, str, -1);
}

int
tcl_reload_scripts(void) {
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
void
tcl_init(void) {
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
void
tcl_deinit(void) {
	deinit_commands();
	deinit_signals();
	Tcl_DeleteInterp(interp);
}

/*
	Print a message using the public message format (to channel)
	Can be from others or ourselves
*/
void
print_message_public(SERVER_REC* server_rec, CHANNEL_REC* channel_rec,
	char* target, char* nick, char* address, char* msg)
{
	// See fe-common/core/fe-messages.c, sig_message_public()
	// and fe-common/irc/fe-irc-messages.c, sig_message_irc_op_public()
	// for the below.

	char* nickmode = channel_get_nickmode(channel_rec, nick);

	// Check if hilight by someone referring to me
	int for_me = nick_match_msg(channel_rec, msg, server_rec->nick);

	// Also can hilight based on address?
	int hilight;
	if (address != NULL) {
		hilight = for_me
			|| hilight_match_nick(server_rec, target, nick, address,
					MSGLEVEL_PUBLIC, msg);
	} else {
		hilight = for_me;
	}

	// If channel is active, we don't need to use the format which includes
	// channel name, such as <@nick:#channel>
	int should_print_channel = channel_rec == NULL
		|| !window_item_is_active((WI_ITEM_REC*) channel_rec);

	// Check if it was us that said this
	int from_me = strcmp(nick, server_rec->nick) == 0;

	// Fix up the message level
	int msg_level = MSGLEVEL_PUBLIC;
	// We don't want to hilight ourselves
	if (!from_me && hilight) {
		msg_level |= MSGLEVEL_HILIGHT;
	}
	
	if (should_print_channel) {
		if (from_me) {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				TXT_OWN_MSG_CHANNEL,
				nick, target, msg, nickmode);
		} else {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				hilight ? TXT_PUBMSG_ME_CHANNEL : TXT_PUBMSG_CHANNEL,
				nick, target, msg, nickmode);
		}
	} else {
		if (from_me) {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				TXT_OWN_MSG,
				nick, msg, nickmode);
		} else {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				hilight ? TXT_PUBMSG_ME : TXT_PUBMSG,
				nick, msg, nickmode);
		}
	}
	g_free_not_null(nickmode);
}
