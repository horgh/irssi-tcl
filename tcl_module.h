#define MODULE_NAME "tcl"

#include <common.h>
#include <core/servers.h>
#include <core/settings.h>
#include <core/levels.h>
#include <core/signals.h>
#include <core/commands.h>
#include <core/queries.h>
#include <core/channels.h>
#include <core/recode.h>
#include <fe-common/core/printtext.h>
#include <fe-common/core/window-items.h>
#include <irc/core/irc.h>
#include <irc/core/irc-commands.h>
#include <irc/core/irc-servers.h>

void init_commands();
void deinit_commands();
static void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item);
void init_signals();
void deinit_signals();
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target);
void time_change();
int tcl_register_commands();
int irssi_dir(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int putserv_raw(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int irssi_print(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int settings_get(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int settings_add_str_tcl(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int interp_init();
int tcl_command(const char *cmd);
const char *tcl_str_result();
const char *tcl_str_error();
int execute(int num, ...);
void irssi_dir_ds (Tcl_DString *dsPtr, char *str);
int tcl_reload_scripts();
void tcl_init(void);
void tcl_deinit(void);
