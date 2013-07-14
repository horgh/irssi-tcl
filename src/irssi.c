// vim: tabstop=2:shiftwidth=2:noexpandtab
/*
 * irssi specific functions.
 */

#include <string.h>

#include "irssi.h"

/*
 * Print a message using the public message format (to channel)
 * Can be from others or ourselves
 */
void
print_message_public(SERVER_REC* server_rec, CHANNEL_REC* channel_rec,
	char* target, char* nick, char* address, char* msg)
{
	// See fe-common/core/fe-messages.c, sig_message_public()
	// and fe-common/irc/fe-irc-messages.c, sig_message_irc_op_public()
	// for the below.

	char* nickmode = channel_get_nickmode(channel_rec, nick);

	// Check if hilight by someone referring to me
	int for_me = nick_match_msg(channel_rec, msg, server_rec->nick);

	// Also can hilight based on address?
	int hilight;
	if (address != NULL) {
		hilight = for_me
			|| hilight_match_nick(server_rec, target, nick, address,
					MSGLEVEL_PUBLIC, msg);
	} else {
		hilight = for_me;
	}

	// If channel is active, we don't need to use the format which includes
	// channel name, such as <@nick:#channel>
	int should_print_channel = channel_rec == NULL
		|| !window_item_is_active((WI_ITEM_REC*) channel_rec);

	// Check if it was us that said this
	int from_me = strcmp(nick, server_rec->nick) == 0;

	// Fix up the message level
	int msg_level = MSGLEVEL_PUBLIC;
	// We don't want to hilight ourselves
	if (!from_me && hilight) {
		msg_level |= MSGLEVEL_HILIGHT;
	}
	
	if (should_print_channel) {
		if (from_me) {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				TXT_OWN_MSG_CHANNEL,
				nick, target, msg, nickmode);
		} else {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				hilight ? TXT_PUBMSG_ME_CHANNEL : TXT_PUBMSG_CHANNEL,
				nick, target, msg, nickmode);
		}
	} else {
		if (from_me) {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				TXT_OWN_MSG,
				nick, msg, nickmode);
		} else {
			printformat_module("fe-common/core", server_rec, target, msg_level,
				hilight ? TXT_PUBMSG_ME : TXT_PUBMSG,
				nick, msg, nickmode);
		}
	}
	g_free_not_null(nickmode);
}
