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
cmd_tcl(const char*, void*, WI_ITEM_REC*);

void
tcl_register_commands(void);
int
interp_init(void);
int
tcl_command(const char*);
const char*
tcl_str_result(void);
const char*
tcl_str_error(void);
int
execute(int num, ...);
void
irssi_dir_ds(Tcl_DString*, const char*);
int
tcl_reload_scripts(void);
void
tcl_init(void);
void
tcl_deinit(void);

#endif
