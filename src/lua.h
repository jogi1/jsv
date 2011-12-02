#ifndef LUA_H
#define LUA_H
qboolean LUA_Init(struct server *server);
void LUA_Cleanup(struct server *server);
qboolean LUA_CallFunction(struct server *server, struct lua *state, struct client *client, char *function);
qboolean LUA_CallFunctionTokens(struct server *server, struct lua *state, struct client *client, char *function, struct tokenized_string *ts, int offset);
qboolean LUA_CallFunctionArguments(struct server *server, struct lua *state, char *function, int retvals, qboolean prefix, char *arguments, ...);
void LUA_Shutdown(struct server *server);
void LUA_GetSpawn(struct server *server, vec3_t *vec);
qboolean LUA_Server_SetVariable(struct server *server, struct lua *state, char *format, ...);
#endif
