#ifndef __TCL_H
#define __TCL_H

#include <tcl.h>

void
tcl_register_commands(void);
int
tcl_interp_init(void);
void
tcl_interp_deinit(void);
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

#endif
