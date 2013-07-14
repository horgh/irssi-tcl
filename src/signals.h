// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __SIGNALS_H
#define __SIGNALS_H

#include "irssi_includes.h"

void
time_change(void);
void
send_text(char*, SERVER_REC*, WI_ITEM_REC*);
void
msg_pub(SERVER_REC*, char*, const char*, const char*,
	const char*);
void
msg_own_pub(SERVER_REC*, char*, char*);
void
server_sendmsg(SERVER_REC*, char*, char*, int);

#endif
