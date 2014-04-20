// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __TCL_COMMANDS_H
#define __TCL_COMMANDS_H

#include <tcl.h>

int
cmd_irssi_dir(ClientData, Tcl_Interp*, int,
	Tcl_Obj* const []);
int
sig_stop(ClientData, Tcl_Interp*, int,
	Tcl_Obj* const []);
int
putserv_raw(ClientData, Tcl_Interp*, int,
	Tcl_Obj* const []);
int
putchan_raw(ClientData, Tcl_Interp*, int,
	Tcl_Obj* const []);
int
emit_message_public(ClientData, Tcl_Interp*,
	int, Tcl_Obj* const []);
int
print_message_public_tcl(ClientData, Tcl_Interp*,
	int, Tcl_Obj* const []);

int
irssi_print(ClientData, Tcl_Interp*, int,
	Tcl_Obj* const []);
int
settings_get_str_tcl(ClientData, Tcl_Interp*,
	int, Tcl_Obj* const []);
int
settings_add_str_tcl(ClientData, Tcl_Interp*,
	int, Tcl_Obj* const []);

int
tcl_command_nicklist_getnicks(ClientData, Tcl_Interp*,
	int, Tcl_Obj* const []);

typedef struct {
	const char* const cmd;
	int (*func)(ClientData, Tcl_Interp*, int, Tcl_Obj* const []);
} TclCmd;

static const TclCmd TclCmdTable[] = {
	{"putserv_raw", putserv_raw},
	{"putchan_raw", putchan_raw},
	{"emit_message_public", emit_message_public},
	{"print_message_public", print_message_public_tcl},
	{"irssi_print", irssi_print},
	{"settings_get_str", settings_get_str_tcl},
	{"settings_add_str", settings_add_str_tcl},
	{"irssi_dir", cmd_irssi_dir},
	{"signal_stop", sig_stop},
	{"nicklist_getnicks", tcl_command_nicklist_getnicks},
	{NULL, NULL}
};

#endif
