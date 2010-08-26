#include <tcl.h>
#include "tcl_module.h"

#define MAX_CMD_LEN 1024

static Tcl_Interp *interp;

int interp_init()
{
	interp = Tcl_CreateInterp();
	if (interp == NULL)
		return -1;

	return 1;
}

int tcl_command(const char *cmd)
{
	if (Tcl_Eval(interp, cmd) == TCL_OK)
		return -1;
	return 1;
}

const char *tcl_result()
{
	return Tcl_GetStringResult(interp);
}

/*
	Concat all strings given in va_list into target with a space between each
	*/
void concat_strs(char *target, int len, int num, ...)
{
	int i;
	char *str;

	va_list vl;
	va_start(vl, num);

	target[0] = '\0';
	for (i = 0; i < num; i++)
	{
		str = va_arg(vl, char *);
		strncat(target, str, len - strlen(target) - 1);
		// don't add " " to end
		if (i < num - 1)
			strncat(target, " ", len - strlen(target) - 1);
	}
	
	va_end(vl);
}

static void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item)
{
	// /tcl reload
	if (strcmp(data, "reload") == 0)
	{
		if(tcl_reload_scripts())
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Scripts reloaded");
		else
			printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Reload failure");
		return;
	}

	printtext(NULL, NULL, MSGLEVEL_CRAP, "Running /tcl: '%s'", data);
	int rc = tcl_command(data);
	const char *res = tcl_result();
	printtext(NULL, NULL, MSGLEVEL_CRAP, "Result: %s", res);
}

void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target)
{
	char *cmd = (char *) malloc(MAX_CMD_LEN);
	concat_strs(cmd, MAX_CMD_LEN, 6, "emit_msg_pub", server->tag, msg, nick, address, target);

	printtext(NULL, NULL, MSGLEVEL_CRAP, "%s %s %s %s %s", server-> tag, msg, nick, address, target);
	int rc = tcl_command(cmd);
	printtext(NULL, NULL, MSGLEVEL_CRAP, "cmd: %s", cmd);
	free(cmd);
	const char *res = tcl_result();
	printtext(NULL, NULL, MSGLEVEL_CRAP, "Result: %s", res);
}

/* commands in tcl interp */
int putserv(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
	if (objc != 3)
	{
		Tcl_Obj *str = Tcl_ObjPrintf("wrong # args: should be \"putserv server_tag text\"");
		Tcl_SetObjResult(interp, str);
		return TCL_ERROR;
	}

	char *server_tag = Tcl_GetStringFromObj(objv[0], NULL);
	SERVER_REC *server = server_find_tag(server_tag);

	char *cmd = Tcl_GetStringFromObj(objv[1], NULL);
//	irc_send_cmdv((IRC_SERVER_REC *) server, cmd);
//	server_command(cmd, server, NULL);
	signal_continue(2, server, cmd);

	return TCL_OK;
}

int tcl_register_commands()
{
	Tcl_CreateObjCommand(interp, "putserv", putserv, NULL, NULL);

	return 1;
}

// XXX deal with path
int tcl_reload_scripts()
{
	if (Tcl_EvalFile(interp, "/home/will/code/irssi_tcl/binds.tcl") == TCL_OK)
		return 1;
	return -1;
}

/*
 * Irssi module load
 */
void tcl_init(void)
{
	module_register(MODULE_NAME, "core");

	if(!interp_init())
	{
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Interp init error");
		return;
	}

	if (!tcl_register_commands())
	{
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Command setup error");
		return;
	}

	if(!tcl_reload_scripts())
	{
		printtext(NULL, NULL, MSGLEVEL_CRAP, "Tcl: Setup error");
		return;
	}

	command_bind("tcl", NULL, (SIGNAL_FUNC) cmd_tcl);

	signal_add("message public", (SIGNAL_FUNC) msg_pub);
}

/*
 * Irssi module unload
 */
void tcl_deinit(void)
{
	command_unbind("tcl", (SIGNAL_FUNC) cmd_tcl);

	Tcl_DeleteInterp(interp);
}
