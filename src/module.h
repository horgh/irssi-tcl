// vim: tabstop=2:shiftwidth=2:noexpandtab

#define MODULE_NAME "tcl"

void
init_commands();
void
deinit_commands();
void
cmd_tcl(const char* data, void* server, WI_ITEM_REC* item);

void
tcl_register_commands();
int
interp_init();
int
tcl_command(const char* cmd);
const char*
tcl_str_result();
const char*
tcl_str_error();
int
execute(int num, ...);
void
irssi_dir_ds(Tcl_DString* dsPtr, char* str);
int
tcl_reload_scripts();
void
tcl_init(void);
void
tcl_deinit(void);

// misc useful functions
void
print_message_public(SERVER_REC* server_rec, CHANNEL_REC* channel_rec,
	char* target, char* nick, char* address, char* msg);
