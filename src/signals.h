void init_signals();
void deinit_signals();
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target);
void msg_own_pub(SERVER_REC* server_rec, char* msg, char* target);
void send_text(char *line, SERVER_REC *server, WI_ITEM_REC *item);
void server_sendmsg(SERVER_REC *server, char *target, char *msg, int type);
void time_change();
