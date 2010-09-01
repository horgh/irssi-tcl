void init_signals();
void deinit_signals();
void msg_pub(SERVER_REC *server, char *msg, const char *nick, const char *address, const char *target);
void server_sendmsg(SERVER_REC *server, char *target, char *msg, int type);
void time_change();
