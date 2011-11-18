#ifndef LUA_H
#define LUA_H
qboolean LUA_Init(struct server *server);
void LUA_Cleanup(struct server *server);
qboolean LUA_CallFunction(struct server *server, struct lua *state, struct client *client, char *function);
qboolean LUA_CallFunctionTokens(struct server *server, struct lua *state, struct client *client, char *function, struct tokenized_string *ts, int offset);
qboolean LUA_CallFunctionArguments(struct server *server, struct lua *state, struct client *client, char *function, char *arguments, ...);
void LUA_Shutdown(struct server *server);
#endif
