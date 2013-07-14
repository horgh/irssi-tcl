// vim: tabstop=2:shiftwidth=2:noexpandtab

#ifndef __IRSSI_INCLUDES_H
#define __IRSSI_INCLUDES_H

// this is to fix a bug apparently.
// see github.com/shabble/irssi-scripts test module.
// another way to silence this seems to be to pass in
// define HAVE_CONFIG_H..
#define UOFF_T_LONG_LONG 1

#include <common.h>

// MODULE_NAME is used in Irssi. we need it defined for some of the
// irssi includes.
#define MODULE_NAME "tcl"

#include <core/settings.h>
#include <core/levels.h>
#include <fe-common/core/printtext.h>
#include <fe-common/core/window-items.h>
#include <irc/core/irc.h>
#include <irc/core/irc-servers.h>
#include <fe-common/core/module-formats.h>
#include <fe-common/core/fe-messages.h>
#include <fe-common/core/hilight-text.h>

#endif
