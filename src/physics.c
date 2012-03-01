#include "server.h"

void Physics_Frame(struct server *server)
{
	int i;
	int x;
	struct client *client;
	struct edict *e;
	vec3_t forward, right, up, end;
	struct trace *trace;

	for (i=0; i<32; i++)
	{
		if (server->clients[i].inuse == false)
			continue;

		Player_Move(server, &server->clients[i], &server->edicts[i+1], &client->ucmd_new);

		/*

		client = &server->clients[i];

		Vector_Angles(server->edicts[i+1].state.angles, forward, right, up);

		Vector_Scale(up, server->frametime * client->ucmd_new.upmove, up);
		Vector_Scale(forward, server->frametime * client->ucmd_new.forwardmove, forward);
		Vector_Scale(right, server->frametime * client->ucmd_new.sidemove, right);

		e = &server->edicts[i+1];

		Vector_Add(end, e->state.origin, up);
		Vector_Add(end, end, forward);
		Vector_Add(end, end, right);

		trace = Trace_Trace(server, NULL, e->state.mins, e->state.maxs, e->state.origin, end, 0, e);
		Vector_Copy(e->state.origin, trace->endpos);
		*/
		/*
		printf("mins: ");
		PRINT_VEC(e->state.mins);
		printf("maxs: ");
		PRINT_VEC(e->state.maxs);
		*/

		/*
		Vector_Add(server->edicts[i+1].state.origin, server->edicts[i+1].state.origin, up);
		Vector_Add(server->edicts[i+1].state.origin, server->edicts[i+1].state.origin, forward);
		Vector_Add(server->edicts[i+1].state.origin, server->edicts[i+1].state.origin, right);
		*/
		for (x=0; x<3; x++)
		{
			server->edicts[i+1].state.angles[x] = client->ucmd_new.angles[x];
		}
	}

	for (i=MAX_CLIENTS+1; i<server->edicts_count; i++)
	{
		e = &server->edicts[i];
		if (e->inuse == false)
			continue;
		if (e->nexttime > server->realtime)
			continue;

		LUA_CallFunctionArguments(server, &server->mod, "handle_entity", 0, false,  "u", e);
	}
}
