// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __SIGNALS_H
#define __SIGNALS_H

#include "irssi_includes.h"

void
time_change(void);

void
send_text(
		char const *,
		SERVER_REC *,
		WI_ITEM_REC *,
		void const *,
		void const *,
		void const *
);

void
msg_pub(
		SERVER_REC*,
		char*,
		const char*,
		const char*,
		const char*,
		void const *
);

void
msg_own_pub(
		SERVER_REC*,
		char*,
		char*,
		void const *,
		void const *,
		void const *
);

void
server_sendmsg(
		SERVER_REC*,
		char*,
		char*,
		void const *,
		void const *,
		void const *
);

#endif
