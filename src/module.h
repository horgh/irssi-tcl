#define MODULE_NAME "tcl"

void init_commands();
void deinit_commands();
void cmd_tcl(const char *data, void *server, WI_ITEM_REC *item);

void tcl_register_commands();
int interp_init();
int tcl_command(const char *cmd);
const char *tcl_str_result();
const char *tcl_str_error();
int execute(int num, ...);
void irssi_dir_ds (Tcl_DString *dsPtr, char *str);
int tcl_reload_scripts();
void tcl_init(void);
void tcl_deinit(void);
