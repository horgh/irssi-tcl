// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __MODULE_H
#define __MODULE_H

void
init_commands(void);
void
deinit_commands(void);
void
init_signals(void);
void
deinit_signals(void);

void
tcl_init(void);
void
tcl_deinit(void);

#ifdef IRSSI_ABI_VERSION
void
tcl_abicheck(int *);
#endif

#endif
