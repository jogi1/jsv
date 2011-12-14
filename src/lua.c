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

static int EDICT_Remove(lua_State *L)
{
	struct edict *edict;
	if (lua_isuserdata(L, 1))
	{
		edict = (struct edict *)lua_touserdata(L, 1);
		if (edict)
		{
			edict->inuse = false;
			printf("removing edict\n");
		}
	}
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
		}
	}
	return 1;
}

static int EDICT_SetMM(lua_State *L)
{
	struct edict *edict;
	int i;

	if (lua_isuserdata(L, 1))
	{
		edict = lua_touserdata(L, 1);
		for (i=0; i<3; i++)
		{
			edict->state.mins[i] = lua_tonumber(L, i+2);
		}
		for (i=3; i<6; i++)
		{
			edict->state.maxs[i-3] = lua_tonumber(L, i+2);
		}
	}
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
	}
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

static int EDICT_SetSkinNum(lua_State *L)
{
	struct edict *edict;
	if (lua_isuserdata(L, 1))
	{
		edict = lua_touserdata(L, 1);
		edict->state.skinnum = lua_tonumber(L, 2);
		lua_pushboolean(L, 1);
	}
	else
		lua_pushboolean(L, 0);
	return 1;
}

static luaL_reg Edict_Functions_Methods[] = 
{
	{"get_unused", EDICT_GetUnused},
	{"remove", EDICT_Remove},
	{"__set_origin", EDICT_SetOrigin},
	{"__set_angles", EDICT_SetAngles},
	{"__set_mm", EDICT_SetMM},
	{"set_modelindex", EDICT_SetModelIndex},
	{"set_baseline", EDICT_SetBaseline},
	{"set_skinnum", EDICT_SetSkinNum},
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

static void LUA_Pushmodel(struct server *server, lua_State *L, int model_number)
{
	vec3_t mins, maxs;
	int i;

	i = model_number;

	if (server->model_precache[model_number] == NULL)
	{
		printf("its null\n");
		lua_pushnil(L);
		return;
	}


	memset(&mins, 0, sizeof(*mins));
	memset(&maxs, 0, sizeof(*maxs));

	if (server->model_precache[model_number][0] == '*')
	{
		i = atoi(server->model_precache[model_number] + 1);
		if (i >= 0 && i < server->map->submodels_count)
		{
			Vector_Copy(mins, server->map->submodels[i].mins);
			Vector_Copy(maxs, server->map->submodels[i].maxs);
		}
	}

	// actual model
	lua_newtable(L);

	lua_pushnumber(L, model_number + 1);
	lua_setfield(L, -2, "index");

	//mins
	lua_pushstring(L,"mins");
	lua_newtable(L);
	lua_pushstring(L,"x");
	lua_pushnumber(L, mins[0]);
	lua_settable(L, -3);
	lua_pushstring(L,"y");
	lua_pushnumber(L, mins[1]);
	lua_settable(L, -3);
	lua_pushstring(L,"z");
	lua_pushnumber(L, mins[2]);
	lua_settable(L, -3);
	lua_settable(L, -3);

	//maxs
	lua_pushstring(L,"maxs");
	lua_newtable(L);
	lua_pushstring(L,"x");
	lua_pushnumber(L, maxs[0]);
	lua_settable(L, -3);
	lua_pushstring(L,"y");
	lua_pushnumber(L, maxs[1]);
	lua_settable(L, -3);
	lua_pushstring(L,"z");
	lua_pushnumber(L, maxs[2]);
	lua_settable(L, -3);
	lua_settable(L, -3);
}

static int SFM_PrecacheModel(lua_State *L)
{
	struct server *server;
	const char *s;
	int i;
	qboolean a;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isstring(L, 2))
		{
			s = luaL_checkstring(L, 2);
			if (lua_isboolean(L, 3))
				a = lua_toboolean(L, 3);
			else
				a = false;

			LUA_Pushmodel(server, L, Server_PrecacheModel(server, (char *)s, a));
			return 1;
		}
	}
	lua_pushnil(L);
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

static int SFM_GetEdictForOnlineModel(lua_State *L)
{
	struct server *server;
	const char *model;
	struct edict *edict;
	if (lua_isuserdata(L, 1))
	{
		server = (struct server *)lua_touserdata(L, 1);
		model = luaL_checkstring(L, 2);
		edict = Server_GetEdictForInlineModel(server, (char *)model);
		if (!edict)
			lua_pushnil(L);
		else
			lua_pushlightuserdata(L, edict);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int SFM_PrintToClient(lua_State *L)
{
	struct server *server;
	struct client *client;
	const char *s;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isuserdata(L, 2))
		{
			client = lua_touserdata(L, 2);
			if (lua_isstring(L, 3))
			{
				s = lua_tostring(L, 3);
				Print_ToClient(client, true, 3, (char *)s);
			}
		}
	}
	return 1;
}

/*
static int SFM_PrintToClientOOB(lua_State *L)
{
	struct server *server;
	struct client *client;
	const char *s;
	qboolean flush;

	if (lua_isuserdata(L, 1))
	{
		server = lua_touserdata(L, 1);
		if (lua_isuserdata(L, 2))
		{
			client = lua_touserdata(L, 2);
			flush = false;
			s = NULL;
			if (lua_isstring(L, 3))
			{
				s = lua_tostring(L, 3);
			}
			else if (lua_isboolean(L, 3))
			{
				flush = lua_toboolean(L, 3);
				if (lua_isstring(L, 4))
					s = lua_tostring(L, 4);
				else
					s = NULL;
			}
			Print_ToClientOOB(server, client, flush, s);
		}
	}
	return 1;
}
*/

static void puttraceonstack(lua_State *L, struct trace *trace)
{
	if (!L || !trace)
		return;

	// trace trable
	lua_newtable(L);

	lua_pushnumber(L, trace->fraction);
	lua_setfield(L, -2, "fraction");

	lua_pushboolean(L, trace->allsolid);
	lua_setfield(L, -2, "allsolid");

	// endpos
	lua_pushstring(L,"endpos");
	lua_newtable(L);
	lua_pushstring(L,"x");
	lua_pushnumber(L, trace->endpos[0]);
	lua_settable(L, -3);
	lua_pushstring(L,"y");
	lua_pushnumber(L, trace->endpos[1]);
	lua_settable(L, -3);
	lua_pushstring(L,"z");
	lua_pushnumber(L, trace->endpos[2]);
	lua_settable(L, -3);
	lua_settable(L, -3);

}

static int SFM_TraceEdict(lua_State *L)
{
	struct server *server;
	struct edict *edict, *passedict;
	struct trace *trace;
	vec3_t start, stop;
	int type;

	if (lua_isuserdata(L, 1))
	{
		server = (struct server *)lua_touserdata(L, 1);
		if (server)
		{
			if (lua_isuserdata(L, 2))
			{
				edict = (struct edict *)lua_touserdata(L, 2);
				if (edict)
				{
					start[0] = lua_tonumber(L, 3);
					start[1] = lua_tonumber(L, 4);
					start[2] = lua_tonumber(L, 5);
					stop[0] = lua_tonumber(L, 6);
					stop[1] = lua_tonumber(L, 7);
					stop[2] = lua_tonumber(L, 8);
					type = lua_tonumber(L, 9);
					passedict = NULL;
					if (lua_isuserdata(L, 10))
						passedict = (struct edict *)lua_touserdata(L, 10);

					trace = Trace_Trace(server, NULL, edict->state.mins, edict->state.maxs, start, stop, type, passedict);
					/*
					printf("start: ");
					PRINT_VEC(start);
					printf("stop : ");
					PRINT_VEC(stop);
					printf("end  : ");
					PRINT_VEC(trace->endpos);
					*/

					puttraceonstack(L, trace);
					free(trace);
					return 1;
				}
			}
		}
	}
	lua_pushnil(L);
	return 1;
}

static luaL_reg Server_Functions_Methods[] = 
{
	{"__precache_sound", SFM_PrecacheSound},
	{"__precache_model", SFM_PrecacheModel},
	{"__get_entities_string", SFM_GetEntitiesString},
	{"__set_map_name", SFM_SetMapName},
	{"__add_lightstyle", SFM_AddLightstyle},
	{"__get_edict_for_inline_model", SFM_GetEdictForOnlineModel},
	{"__print_to_client", SFM_PrintToClient},
	{"__trace_edict", SFM_TraceEdict},
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
		lua_pop(state->L, 1);
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

	if (luaL_loadfile(state->L, "lua/helpers/edict.lua"))
	{
		Log_Print(server->log, log_debug, "LUA: error loading \"helpers/edict.lua\": %s", lua_tostring(state->L, -1));
		return false;
	}

	if (lua_pcall(state->L, 0, LUA_MULTRET, 0))
	{
		Log_Print(server->log, log_debug, "LUA: error loading \"helpers/edict.lua\": %s", lua_tostring(state->L, -1));
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

qboolean LUA_CallFunctionArguments(struct server *server, struct lua *state, char *function, int retvals, qboolean prefix, char *arguments, ...)
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

	if (server->debug_lua_stack)
		printf("CFA before: %i\n", lua_gettop(state->L));

	snprintf(buffer, sizeof(buffer), "%s%s", prefix ? "FUNC_" : "", function);
	lua_getglobal(state->L, buffer);
	if (LUA_TFUNCTION != lua_type(state->L, -1))
	{
		printf("LUA: no function \"%s\"\n", function);
		if (server->debug_lua_stack)
			printf("CFA after: %i\n", lua_gettop(state->L));
		lua_pop(state->L, 1);	// error and table
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
		if (server->debug_lua_stack)
			printf("CFA after: %i\n", lua_gettop(state->L));
		return false;
	}

	if (server->debug_lua_stack)
		printf("CFA after: %i\n", lua_gettop(state->L));
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
	LUA_CallFunctionArguments(server, &server->mod, "get_spawn", 1, false, NULL);
	lua_getvector(server->mod.L, vec);
	lua_pop(server->mod.L, 1);
}

qboolean LUA_Server_SetVariable(struct server *server, struct lua *state, char *format, ...)
{
	char *string, *sval, *c;
	int ival;
	double dval;
	va_list argptr;

	if (!server || !state || !format)
		return false;

	if (server->debug_lua_stack)
		printf("SSV before: %i\n", lua_gettop(state->L));
	lua_getglobal(state->L, "server");
	if (LUA_TTABLE != lua_type(state->L, 1))
	{
		Log_Print(server->log, log_debug, "LUA: server is not a table.");
		if (server->debug_lua_stack)
			printf("SSV after: %i\n", lua_gettop(state->L));
		return false;
	}

	va_start(argptr, format);

	c = format;

	while (*c)
	{
		string = (char *)va_arg(argptr, int *);
		if (!string)
		{
			va_end(argptr);
			return false;
		}

		switch (*c)
		{
			case 'd':
			case 'f':
				dval = (double)va_arg(argptr, double);
				lua_pushnumber(state->L, dval);
				break;
			case 's':
				sval = (char *)va_arg(argptr, int *);
				lua_pushstring(state->L, sval);
				break;
			case 'i':
				ival = (int)va_arg(argptr, int);
				lua_pushnumber(state->L, ival);
				break;
		}
		lua_setfield(state->L, -2, string);
		c++;
	}

	lua_pop(state->L, 1);
	va_end(argptr);

	if (server->debug_lua_stack)
		printf("SSV after: %i\n", lua_gettop(state->L));

	return true;
}
