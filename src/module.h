// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __MODULE_H
#define __MODULE_H

#include <tcl.h>
#include "irssi_includes.h"

void
init_commands(void);
void
deinit_commands(void);
void
cmd_tcl(const char* data, void* server, WI_ITEM_REC* item);

void
tcl_register_commands(void);
int
interp_init(void);
int
tcl_command(const char* cmd);
const char*
tcl_str_result(void);
const char*
tcl_str_error(void);
int
execute(int num, ...);
void
irssi_dir_ds(Tcl_DString* dsPtr, const char* str);
int
tcl_reload_scripts(void);
void
tcl_init(void);
void
tcl_deinit(void);

// misc useful functions
void
print_message_public(SERVER_REC* server_rec, CHANNEL_REC* channel_rec,
	char* target, char* nick, char* address, char* msg);

#endif
