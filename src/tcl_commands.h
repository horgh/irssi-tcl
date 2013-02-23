// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __TCL_COMMANDS_H
#define __TCL_COMMANDS_H

#include <tcl.h>

int
irssi_dir(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[]);
int
sig_stop(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[]);
int
putserv_raw(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[]);
int
putchan_raw(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[]);
int
emit_message_public(ClientData clientData, Tcl_Interp* interp,
	int objc, Tcl_Obj* const objv[]);
int
print_message_public_tcl(ClientData clientData, Tcl_Interp* interp,
	int objc, Tcl_Obj* const objv[]);

int
irssi_print(ClientData clientData, Tcl_Interp* interp, int objc,
	Tcl_Obj* const objv[]);
int
settings_get_str_tcl(ClientData clientData, Tcl_Interp* interp,
	int objc, Tcl_Obj* const objv[]);
int
settings_add_str_tcl(ClientData clientData, Tcl_Interp* interp,
	int objc, Tcl_Obj* const objv[]);

typedef struct {
	const char* cmd;
	const void* func;
} TclCmd;

static const TclCmd TclCmdTable[] = {
	{"putserv_raw", putserv_raw},
	{"putchan_raw", putchan_raw},
	{"emit_message_public", emit_message_public},
	{"print_message_public", print_message_public_tcl},
	{"irssi_print", irssi_print},
	{"settings_get_str", settings_get_str_tcl},
	{"settings_add_str", settings_add_str_tcl},
	{"irssi_dir", irssi_dir},
	{"signal_stop", sig_stop},
	{NULL, NULL}
};

#endif
