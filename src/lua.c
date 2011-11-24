#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

static int EDICT_GetUnused(lua_State *L)
{
	struct server *server;
	struct edict *edict;
	if (lua_isuserdata(L, 1))
	{
		server = (struct server *)lua_touserdata(L, 1);
		edict = Server_GetFreeEdict(server);
		if (!edict)
			lua_pushnil(L);
		else
			lua_pushlightuserdata(L, edict);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int EDICT_SetBaseline(lua_State *L)
{
	struct edict *edict;

	if (lua_isuserdata(L, 1))
	{
		edict = (struct edict *)lua_touserdata(L, 1);
		if (!edict)
			return 0;
		Server_EdictCreateBaseline(edict);
	}
	return 0;
}

static int EDICT_SetOrigin(lua_State *L)
{
	struct edict *edict;
	int i;
	if (lua_isuserdata(L, 1))
	{
		edict = lua_touserdata(L, 1);
		for (i=0; i<3; i++)
		{
			edict->state.origin[i] = lua_tonumber(L, i+2);
			//printf("%f\n", edict->state.origin[i]);
		}
		lua_pushboolean(L, 1);
	}
	else
		lua_pushboolean(L, 0);
	return 1;
}

static int EDICT_SetAngles(lua_State *L)
{
	struct edict *edict;
	int i;
	if (lua_isuserdata(L, 1))
	{
		edict = lua_touserdata(L, 1);
		for (i=0; i<3; i++)
			edict->state.angles[i] = lua_tonumber(L, i+2);
		lua_pushboolean(L, 1);
	}
	else
		lua_pushboolean(L, 0);
	return 1;
}

static int EDICT_SetModelIndex(lua_State *L)
{
	struct edict *edict;
	if (lua_isuserdata(L, 1))
	{
		edict = lua_touserdata(L, 1);
		edict->state.model_index = lua_tonumber(L, 2);
		lua_pushboolean(L, 1);
	}
	else
		lua_pushboolean(L, 0);
	return 1;
}

static luaL_reg Edict_Functions_Methods[] = 
{
	{"get_unused", EDICT_GetUnused},
	{"set_origin", EDICT_SetOrigin},
	{"set_angles", EDICT_SetAngles},
	{"set_modelindex", EDICT_SetModelIndex},
	{"set_baseline", EDICT_SetBaseline},
	{0,0}
};

static luaL_reg Edict_Functions_Meta [] =
{
	{0, 0}
};

static int SFM_PrecacheSound(lua_State *L)
{
	struct server *server;
	const char *s;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isstring(L, 2))
		{
			s = luaL_checkstring(L, 2);
			lua_pushnumber(L, Server_PrecacheSound(server, (char *)s));
			return 1;
		}
	}
	lua_pushnumber(L, -1);
	return 1;
}

static int SFM_PrecacheModel(lua_State *L)
{
	struct server *server;
	const char *s;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isstring(L, 2))
		{
			s = luaL_checkstring(L, 2);
			lua_pushnumber(L, Server_PrecacheModelNet(server, (char *)s));
			return 1;
		}
	}
	lua_pushnumber(L, -1);
	return 1;
}

static int SFM_SetMapName(lua_State *L)
{
	struct server *server;
	const char *s;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isstring(L, 2))
		{
			s = luaL_checkstring(L, 2);
			if (server->map->name)
				free(server->map->name);
			server->map->name = strdup(s);
			return 0;
		}
	}
	return 0;
}

static int SFM_GetEntitiesString(lua_State *L)
{
	struct server *server;
	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		lua_pushstring(L, server->map->entity_string);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int SFM_AddLightstyle(lua_State *L)
{
	struct server *server;
	const char *s;
	int i;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isstring(L, 2))
		{
			s = luaL_checkstring(L, 2);
			if (!s)
				lua_pushnumber(L, -1);
			else
			{
				i = Server_AddLightstyle(server, (char *) s);
				lua_pushnumber(L, i);
			}
			return 1;
		}
	}
	lua_pushnumber(L, -1);
	return 1;
}

static luaL_reg Server_Functions_Methods[] = 
{
	{"__precache_sound", SFM_PrecacheSound},
	{"__precache_model", SFM_PrecacheModel},
	{"__get_entities_string", SFM_GetEntitiesString},
	{"__set_map_name", SFM_SetMapName},
	{"__add_lightstyle", SFM_AddLightstyle},
	{0,0}
};

static luaL_reg Server_Functions_Meta [] =
{
	{0, 0}
};

// stolen from awesome tiling wm
static void
LUA_RegisterFunctions(const char *name,lua_State *L, 
             const struct luaL_reg methods[],
             const struct luaL_reg meta[])                                                                                                                
{
    luaL_newmetatable(L, name);                                        /* 1 */
    lua_pushvalue(L, -1);           /* dup metatable                      2 */
    lua_setfield(L, -2, "__index"); /* metatable.__index = metatable      1 */

    luaL_register(L, NULL, meta);                                      /* 1 */
    luaL_register(L, name, methods);                                   /* 2 */
    lua_pushvalue(L, -1);           /* dup self as metatable              3 */
    lua_setmetatable(L, -2);        /* set self as metatable              2 */
    lua_pop(L, 2);
}

qboolean LUA_StateInit(struct server *server, struct lua *state, char *in_script, qboolean main_script)
{
	char script[1024];

	if (state == NULL || in_script == NULL)
	{
		return false;
	}

	if (main_script)
		snprintf(script, sizeof(script), "lua/%s.lua", in_script);
	else
		snprintf(script, sizeof(script), "lua/mod/%s.lua", in_script);

	state->L = lua_open();
	if (state->L == NULL)
	{
		Log_Print(server->log, log_debug, "LUA: lua_open failed for \"%s\".", script);
		return false;
	}

	luaL_openlibs(state->L);

	// set paths for mod, main script
	lua_getglobal(state->L, "package");
	if (LUA_TTABLE != lua_type(state->L, 1))
	{
		Log_Print(server->log, log_debug, "LUA: package is not a tablele.");
		return false;
	}

	lua_getfield(state->L, 1, "path");
	if (LUA_TSTRING != lua_type(state->L, 2))
	{
		Log_Print(server->log, log_debug, "LUA: package.path is not a string.");
		lua_pop(state->L, 1);
		return false;
	}

	lua_pushliteral(state->L, ";lua/helpers/?.lua;lua/helpers/?/init.lua");
	lua_concat(state->L, 2);

	lua_setfield(state->L, 1, "path");
	lua_pop(state->L, 1);

	LUA_RegisterFunctions("server", state->L, Server_Functions_Methods, Server_Functions_Meta);
	LUA_RegisterFunctions("edict", state->L, Edict_Functions_Methods, Edict_Functions_Meta);

	// load the server helper
	if (luaL_loadfile(state->L, "lua/helpers/server.lua"))
	{
		Log_Print(server->log, log_debug, "LUA: error loading \"helpers/server.lua\": %s", lua_tostring(state->L, -1));
		return false;
	}

	if (lua_pcall(state->L, 0, LUA_MULTRET, 0))
	{
		Log_Print(server->log, log_debug, "LUA: error loading \"helpers/server.lua\": %s", lua_tostring(state->L, -1));
		return false;
	}

	// load the actual script
	if (luaL_loadfile(state->L, script))
	{
		Log_Print(server->log, log_debug, "LUA: error loading \"%s\": %s", script, lua_tostring(state->L, -1));
		return false;
	}

	if (lua_pcall(state->L, 0, LUA_MULTRET, 0))
	{
		Log_Print(server->log, log_debug, "LUA: error loading \"%s\": %s", script, lua_tostring(state->L, -1));
		return false;
	}


	// set the server pointer

	lua_getglobal(state->L, "server");
	if (LUA_TTABLE != lua_type(state->L, 1))
	{
		Log_Print(server->log, log_debug, "LUA: error setting server pointers.");
		return false;
	}

	lua_pushlightuserdata(state->L, server);
	lua_setfield(state->L, -2, "__pointer");
	lua_pop(state->L, 1);


	return true;
}

qboolean LUA_Init(struct server *server)
{
	if (server->main_script == NULL)
	{
		printf("LUA: no main script\n");
		return false;
	}

	if (LUA_StateInit(server, &server->main, server->main_script, true) == false)
		return false;

	if (server->mod_script)
		return (LUA_StateInit(server, &server->mod, server->mod_script, false));
	else
		return (LUA_StateInit(server, &server->mod, "basic", false));

	return true;
}

qboolean LUA_CallFunctionTokens(struct server *server, struct lua *state, struct client *client, char *function, struct tokenized_string *ts, int offset)
{
	char buffer[1024];
	int i, x;

	if (!server || !state || !function)
		return false;

	snprintf(buffer, sizeof(buffer), "FUNC_%s", function);
	lua_getglobal(state->L, buffer);
	if (LUA_TFUNCTION != lua_type(state->L, -1))
	{
		lua_pop(state->L, 1);
	//	printf("LUA: no function \"%s\"\n", function);
		return false;
	}
	x = 0;
	lua_pushlightuserdata(state->L, (void *)server);
	x++;
	if (client)
	{
		lua_pushlightuserdata(state->L, (void *)client);
		x++;
	}

	if (ts)
		for (i=offset; i<ts->count; i++,x++)
			lua_pushstring(state->L, ts->tokens[i]);

	if (lua_pcall(state->L, x, LUA_MULTRET, 0))
	{
		printf("LUA: error %s\n", lua_tostring(state->L, -1));
		return false;
	}
	return true;
}

qboolean LUA_CallFunction(struct server *server, struct lua *state, struct client *client, char *function)
{
	char buffer[1024];
	int x;

	if (!server || !state || !function)
		return false;

	snprintf(buffer, sizeof(buffer), "FUNC_%s", function);
	lua_getglobal(state->L, buffer);
	if (LUA_TFUNCTION != lua_type(state->L, -1))
	{
		printf("LUA: no function \"%s\"\n", function);
		return false;
	}
	x = 0;
	lua_pushlightuserdata(state->L, (void *)server);
	x++;
	if (client)
	{
		lua_pushlightuserdata(state->L, (void *)client);
		x++;
	}

	if (lua_pcall(state->L, x, LUA_MULTRET, 0))
	{
		printf("LUA: error %s\n", lua_tostring(state->L, -1));
		return false;
	}
	return true;
}

qboolean LUA_CallFunctionArguments(struct server *server, struct lua *state, char *function, int retvals, char *arguments, ...)
{
	char buffer[1024];
	char *c;
	int i, x;
	va_list argptr;
	char *s;
	float f;
	void *u;

	if (!server || !state || !function)
		return false;

//	printf("CFA before: %i\n", lua_gettop(state->L));

	snprintf(buffer, sizeof(buffer), "FUNC_%s", function);
	lua_getglobal(state->L, buffer);
	if (LUA_TFUNCTION != lua_type(state->L, -1))
	{
		printf("LUA: no function \"%s\"\n", function);
		return false;
	}
	x = 0;
	lua_pushlightuserdata(state->L, (void *)server);
	x++;

	if (arguments)
	{
		va_start(argptr, arguments);
		c = arguments;
		while (*c)
		{
			switch(*c)
			{
				case 'u':
					u = (void *)va_arg(argptr, void *);
					lua_pushlightuserdata(state->L, u);
					x++;
					break;
				case 'i':
					i = (int)va_arg(argptr, int);
					lua_pushnumber(state->L, i);
					x++;
					break;
				case 's':
					s = (char *)va_arg(argptr, int *);
					lua_pushstring(state->L, s);
					x++;
					break;
				case 'S':
					s = (char *)va_arg(argptr, int *);
					x++;
					i = (int)va_arg(argptr, int);
					lua_pushlstring(state->L, s, i);
					break;
				case 'f':
					f = (float)va_arg(argptr, int);
					lua_pushnumber(state->L, f);
					x++;
					break;
			}
			c++;
		}
		va_end(argptr);
	}

	if (retvals == -1)
		retvals = LUA_MULTRET;

	if (lua_pcall(state->L, x, retvals, 0))
	{
		printf("LUA error: \'%s\' %i\n", lua_tostring(state->L, -1), x);
		return false;
	}

//	printf("CFA after: %i\n", lua_gettop(state->L));
	return true;
}

void LUA_Shutdown(struct server *server)
{
	if (server->main.L)
		lua_close(server->main.L);
	if (server->mod.L)
		lua_close(server->mod.L);
}

void lua_getvector(lua_State *L, vec3_t *vec)
{
	float f;
	if (LUA_TTABLE == lua_type(L, -1))
	{
		lua_getfield(L, -1, "x");
		f = lua_tonumber(L, -1);
		(*vec)[0] = f;
		lua_pop(L, 1);
		lua_getfield(L, -1, "y");
		f = lua_tonumber(L, -1);
		(*vec)[1] = f;
		lua_pop(L, 1);
		lua_getfield(L, -1, "z");
		f = lua_tonumber(L, -1);
		(*vec)[2] = f;
		lua_pop(L, 1);
	}
}

void LUA_GetSpawn(struct server *server, vec3_t *vec)
{
	LUA_CallFunctionArguments(server, &server->mod, "get_spawn", 1, NULL);
	lua_getvector(server->mod.L, vec);
	lua_pop(server->mod.L, 1);
}
