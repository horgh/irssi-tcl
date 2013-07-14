// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __MODULE_H
#define __MODULE_H

#include "irssi_includes.h"

void
init_commands(void);
void
deinit_commands(void);
void
cmd_tcl(const char*, void*, WI_ITEM_REC*);

void
tcl_init(void);
void
tcl_deinit(void);

#endif
