#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "server.h"

static qboolean CMD_isinvalidspawncount(struct server *server, struct client *client, int sc)
{
	if (server->spawn_count != sc)
	{
		CMD_New(server, client, NULL);
		return true;
	}
	return false;
}

void CMD_Say(struct server *server, struct client *client, struct tokenized_string *ts)
{
	char buffer[4096];
	int k;
	int i;

	k = 0;
	k += snprintf(buffer + k, sizeof(buffer) -k, "%s%s:", client->spectator ? "[SPEC] " : "" ,client->name);

	for (i=1; i<ts->count; i++)
		k += snprintf(buffer + k, sizeof(buffer) -k, " %s", ts->tokens[i]);
	snprintf(buffer + k, sizeof(buffer) -k, "\n");

	Print_ToAll(server, true, 3, buffer);
	Log_Print(server->log, log_main, buffer);
}

void CMD_New(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int playernum;

	playernum = client - server->clients;
	if (client->spectator)
		playernum |= 128;

	Client_Write(client, "biisbs",
			svc_serverdata, server->protocol_version, server->spawn_count,
			"qw", playernum, server->map->name ? server->map->name : "somethings not right");
	Client_Write(client, "ffffffffff", 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 ,1.0 , 1.0, 1.0);
	Client_Write(client, "bs", svc_stufftext, "fullserverinfo \"cunt!\"\n");
	Client_Write(client, "bb", svc_cdtrack, 0);

	if (0)
	{
		int i;
		for (i=0; i<client->message.position; i++)
			printf("%i: \"%c\" %i\n", i, client->message.data[i], client->message.data[i]);
	}
}

void CMD_Soundlist(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int i;
	int n, sc;
	char **s;

	sc = atoi(ts->tokens[1]);
	if (CMD_isinvalidspawncount(server, client, sc))
		return;

	n = atoi(ts->tokens[2]);
	if (n >= MAX_SOUNDS)
	{
		Client_Write(client, "bbs", svc_print, 2, "invalid soundlist index!.\n");
		return;
	}

	i = client->message.position;
//	snprintf(buffer, sizeof(buffer), "cmd soundlist %s %s\n", "2", ts->tokens[2]);
//	Client_Write(client, "bs", svc_stufftext, buffer);
	Client_Write(client, "bb", svc_soundlist, n);
	for (s = server->sound_precache + n; *s && client->message.position < (MAX_MSGLEN/2); s++, n++)
	{
		Client_Write(client, "s", *s);
	}
	Client_Write(client, "b", 0);
	if (*s)
		Client_Write(client, "b", n);
	else
		Client_Write(client, "b", 0);
//
	if (0)
	{
		for (; i<client->message.position; i++)
			printf("%i: \"%c\" %i\n", i, client->message.data[i], client->message.data[i]);
	}
}

void CMD_Modellist(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int i;
	int n, sc;
	char **s;

	sc = atoi(ts->tokens[1]);
	if (CMD_isinvalidspawncount(server, client, sc))
		return;

	n = atoi(ts->tokens[2]);
	if (n >= MAX_MODELS)
	{
		Client_Write(client, "bbs", svc_print, 2, "invalid modellist index!.\n");
		return;
	}

	i = client->message.position;
//	snprintf(buffer, sizeof(buffer), "cmd modellist %s %s\n", "2", ts->tokens[2]);
//	Client_Write(client, "bs", svc_stufftext, buffer);
	Client_Write(client, "bb", svc_modellist, n);
	for (s = server->model_precache + n; *s && client->message.position < (MAX_MSGLEN/2); s++, n++)
	{
		Client_Write(client, "s", *s);
	}
	Client_Write(client, "b", 0);
	if (*s)
		Client_Write(client, "b", n);
	else
		Client_Write(client, "b", 0);
//
	if (0)
	{
		for (; i<client->message.position; i++)
			printf("%i: \"%c\" %i\n", i, client->message.data[i], client->message.data[i]);
	}
}

void CMD_Prespawn(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int sc;
	int buf;
	char buffer[1024];


	sc = atoi(ts->tokens[1]);
	if (CMD_isinvalidspawncount(server, client, sc))
		return;

	sc = atoi(ts->tokens[3]);
	if (sc != server->map->checksum2)
	{
		Print_Console("client has wrong map checksum\n");
		return;
	}

	buf = atoi(ts->tokens[2]);

	Client_WriteBuffer(client, &server->signon_buffers[server->signon_buffers_count], false);

	buf++;
	if (server->signon_buffers_count == buf)
	{
		snprintf(buffer, sizeof(buffer), "cmd spawn %i 0\n", server->spawn_count);
		Client_Write(client, "cs", svc_stufftext, buffer);
	}
	else
	{
		snprintf(buffer, sizeof(buffer), "cmd prespawn %i %i\n", server->spawn_count, buf);
		Client_Write(client, "cs", svc_stufftext, buffer);
	}
}

void CMD_Spawn(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int sc, i;
	int x;

	sc = atoi(ts->tokens[1]);
	if (CMD_isinvalidspawncount(server, client, sc))
		return;

	i = atoi(ts->tokens[2]);

	if (i>= MAX_CLIENTS)
	{
		Print_ToClient(client, false, 3, "invalid client start\n");
		Server_DropClient(server, client);
		return;
	}

	Server_FullClientUpdateToClient(server, client);
	Server_FullClientUpdate(server, client);

	// send lightstyles
	for (i=0; i<server->lightstyles_index; i++)
		Client_WriteReliable(client, "ccs", svc_lightstyle, (char)i, server->lightstyles[i]);

#warning really fix this!

	Client_WriteReliable(client, "cs", svc_stufftext, "skins\n");
}

void CMD_Begin(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int sc;

	sc = atoi(ts->tokens[1]);
	if (CMD_isinvalidspawncount(server, client, sc))
		return ;

	Client_WriteReliable(client, "ccs", svc_print, 3, "welcome to my shitty server that doesnt do anything... yet!\n");

	client->state = cs_spawned;
}

void CMD_Drop(struct server *server, struct client *client, struct tokenized_string *ts)
{
	Server_DropClient(server, client);
}

struct setinfo_table
{
	char *key;
	qboolean (*function)(struct server *server, struct client *client, char *new);
};

static struct setinfo_table setinfo_table[] =
{
	{"name", &Server_ClientChangeName},
	{"team", NULL},
	{"spectator", NULL},
	{"skin", NULL},
	{"topcolor", NULL},
	{"bottomcolor", NULL},
	{NULL}
};

char *setinfo_nogos[] =
{
	"\\",
	"&r",
	"&c",
	NULL
};

void CMD_Setinfo(struct server *server, struct client *client, struct tokenized_string *ts)
{
	int i, c;

	if (!server || !client || !ts)
		return;

	if (ts->count != 3)
		return;

	if (ts->tokens[1][0] == '*')
		return;

	for (i=0; setinfo_nogos[i] ;i++)
		if (strstr(ts->tokens[1], setinfo_nogos[i]) || strstr(ts->tokens[2], setinfo_nogos[i]))
			return;

	for (i=0; i<(sizeof(setinfo_table)/sizeof(*setinfo_table))  ;i++)
	{
		if (String_Compare(setinfo_table[i].key, ts->tokens[1]))
		{
			c = client - server->clients;
			SendReliable_ToAll(server, "ccss", svc_setinfo, c, ts->tokens[1], ts->tokens[2]);
			if (setinfo_table[i].function)
				setinfo_table[i].function(server, client, ts->tokens[2]);
			break;
		}
	}

	Info_SetValue(client->userinfo, ts->tokens[1], ts->tokens[2]);
}

void CMD_Download(struct server *server, struct client *client, struct tokenized_string *ts)
{
	if (!server || !client || !ts)
		return;

	Print_ToClient(client, true, 3, "downloading not yet implemented.\n");
	Client_WriteReliable(client, "cSc", svc_download, -1, 0);
}
