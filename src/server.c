#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>


#warning fix networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "server.h"

struct server_handles
{
	struct server *server[32];
};

struct server_handles server_handles;

struct edict *Server_GetFreeEdict(struct server *server)
{
	int i;

	if (!server)
		return NULL;

	for (i=0; i<server->edicts_count; i++)
	{
		if (server->edicts[i].inuse == false)
		{
			server->edicts[i].inuse = true;
			server->edicts[i].state.number = i;
			server->edicts[i].baseline.number = i;
			return &server->edicts[i];
		}
	}

#warning maybe should shutdown the server here
	if (server->edicts_count == MAX_EDICTS)
		return NULL;

	server->edicts[server->edicts_count].inuse = true;
	server->edicts[i].state.number = i;
	server->edicts[i].baseline.number = i;
	server->edicts_count++;
	return &server->edicts[(server->edicts_count - 1)];
}

#define MAX_USERID 99
int Server_CreateUserID(struct server *server)
{
	if (!server)
		return -1;

	server->userid++;
	if (server->userid <= MAX_USERID)
		server->userid = 0;

	return server->userid;
}
void Server_EdictCreateBaseline(struct edict *edict)
{
	if (!edict)
		return;
	memcpy(&edict->baseline, &edict->state, sizeof(struct entity_state));
}

/*
 * cleans up and frees the server struct
 */
static void Server_Cleanup(struct server *server)
{
	int i;

	Model_MapFree(server->map);
	LUA_Shutdown(server);
	NET_Shutdown(server->net);
	free(server->ip);
	free(server->data_dir);
	free(server->main_script);

	for (i=0; i<server->lightstyles_index; i++)
	{
		free(server->lightstyles[i]);
	}

	for (i=0; i<server->model_precache_index; i++)
	{
		free(server->model_precache[i]);
	}

	for (i=0; i<server->sound_precache_index; i++)
	{
		free(server->sound_precache[i]);
	}

	for (i=0; i<MAX_CLIENTS; i++)
	{
		Info_Clean(server->clients[i].userinfo);
	}

	if (server->log)
		server->log->run = false;

	free(server->map_filename);
	pthread_join(server->log_thread, NULL);
	free(server);
}

int print_packets;

static void Server_SignalHandler(int sig)
{
	int i;

	if (sig == SIGQUIT)
	{
		for (i=0; i<32; i++)
		{
			if (server_handles.server[i])
			{
				printf("quit via signal: SIGQUIT\n");
				server_handles.server[i]->run = false;
			}
		}
		return;
	}

	return;

	if (sig == SIGQUIT)
	{
		print_packets = !print_packets;
		return;
	}
}

static qboolean Server_DefaultArguments(struct server *server)
{
	server->protocol_version = 28;

	if (!server->data_dir)
	{
		server->data_dir = strdup("data");
		if (server->data_dir == NULL)
		{
			Log_Print(server->log, log_debug, "%s - data_dir strdup failed.", DEBUG_INFO);
			return false;
		}
	}

	if (!server->main_script)
	{
		server->main_script = strdup("main");
		if (server->main_script == NULL)
		{
			Log_Print(server->log, log_debug, "%s - main script strdup failed.", DEBUG_INFO);
			return false;
		}
	}

	return true;
}

static qboolean Server_HandleArguments(struct server *server, int argc, char *argv[])
{
	int i;

	for (i=0; i<argc;i++)
	{
		if (String_Compare(argv[i], "--port") || String_Compare(argv[i], "-p"))
		{
			if (i+1 < argc)
			{
				server->port = atoi(argv[i+1]);
				i++;
			}
		}
		else if (String_Compare(argv[i], "--ip") || String_Compare(argv[i], "-i"))
		{
			if (i+1 < argc)
			{
				if (String_CheckOnly(argv[i+i], "0123456789."))
				{
					server->ip = strdup(argv[i+1]);
					if (server->ip == NULL)
					{
						Log_Print(server->log, log_main, "allocating \"ip\" failed, default fallback will be used.\n");
					}
					i++;
				}
			}
		}
		else if (String_Compare(argv[i], "--script") || String_Compare(argv[i], "-s"))
		{
			if (i+1 < argc)
			{
				server->main_script = strdup(argv[i+1]);
				if (server->ip == NULL)
				{
					Log_Print(server->log, log_main, "allocating \"script\" failed, default fallback will be used.\n");
				}
				i++;
			}
		}
		else if (String_Compare(argv[i], "--mod_script") || String_Compare(argv[i], "-ms"))
		{
			if (i+1 < argc)
			{
				server->mod_script = strdup(argv[i+1]);
				if (server->ip == NULL)
				{
					Log_Print(server->log, log_main, "allocating \"mod_script\" failed, default fallback will be used.\n");
				}
				i++;
			}
		}
		else if (String_Compare(argv[i], "--datadir") || String_Compare(argv[i], "-dd"))
		{
			if (i+1 < argc)
			{
				server->data_dir = strdup(argv[i+1]);
				if (server->data_dir == NULL)
				{
					Log_Print(server->log, log_main, "allocating \"data_dir\" failed, default fallback will be used.\n");
				}
				i++;
			}
		}
		else if (String_Compare(argv[i], "--disable_log"))
		{
			server->disable_log = true;
		}
	}
	return true;
}

static void Server_HandleChallenge(struct server *server, struct packet *packet)
{
	unsigned char *p;
	char challenge[64];
	int len, i, x;

	if (packet == NULL)
		return;

	for (i=0; i<MAX_CHALLENGES; i++)
	{
		if (server->challenges[i].challenge == 0)
			continue;

		if (NET_CompareAdr(&server->challenges[i].address, &packet->address))
		{
			Log_Print(server->log, log_main, "already got a challenge from \"%s\"\n", "replaceme with something usefull");
			return;
		}
	}

	x = (rand() << 16) ^rand();
	snprintf(challenge, sizeof(challenge), "%i", x);
	p = Packet_Create(&len, "ics", HEADER_ID, S2C_CHALLENGE, challenge);
	if (p)
	{

		server->challenges[server->current_challenge].time = server->realtime;
		server->challenges[server->current_challenge].challenge = x;
		memcpy(&server->challenges[server->current_challenge].address, &packet->address, sizeof(struct netaddr));
		Log_Print(server->log, log_debug, "sending challenge: %s - %i", challenge, NET_Send(server->net, p, len, &packet->address));
		free(p);
	}
}

void Server_DropClient(struct server *server, struct client *client)
{
	Client_WriteReliable(client, "c", svc_disconnect);
	Info_Clean(client->userinfo);
	memset(client, 0, sizeof(*client));
}

static void Client_SetupUserinfo(struct server *server, struct client *client, char *userinfo)
{
	char *s;

	client->userinfo = Info_FromString(userinfo);
	s = Info_GetValue(client->userinfo, "name");
	if (s)
		snprintf(client->name, CLIENT_NAME_MAX, "%s", s);
	else
		Log_Print(server->log, log_debug, "%s - could not set name!", DEBUG_INFO);
}

static void Server_HandleConnect(struct server *server, struct packet *packet, struct tokenized_string *ts)
{
	int i, x, challenge;
	struct client *client;
	char *p, *c;
	int p_size;

	// check protocol
	if (atoi(ts->tokens[1]) != 28)
	{
		Log_Print(server->log, log_main, "wrong protocol version.\n");
		p = (char *)Packet_Create(&p_size, "ics", HEADER_ID, A2C_PRINT, "\nwrong protocol version.\n");
		NET_Send(server->net, p, p_size, &packet->address);
		free(p);
		return;
	}

	// check challenge
	challenge = atoi(ts->tokens[3]);
	if (challenge == 0)
		return;
	for(i=0; i<MAX_CHALLENGES; i++)
	{
		if (server->challenges[i].challenge == challenge)
			break;
	}
	if (i == MAX_CHALLENGES)
	{
		Log_Print(server->log, log_main, "wrong challenge\n");
		p = (char *)Packet_Create(&p_size, "ics", HEADER_ID, A2C_PRINT, "\nwrong challenge.\n");
		NET_Send(server->net, p, p_size, &packet->address);
		free(p);
		return;
	}

	x = i;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (server->clients[i].inuse == false)
		{
			client = &server->clients[i];
			break;
		}
	}

	client->inuse = true;
	client->id = Server_CreateUserID(server);
	memcpy(&client->address, &packet->address, sizeof(struct netaddr));
	client->challenge = challenge;
	client->qport = atoi(ts->tokens[2]);
	Log_Print(server->log, log_debug, "client info: %s", ts->tokens[4]);
	Client_SetupUserinfo(server, client, ts->tokens[4]);

	if ((c=Info_GetValue(client->userinfo, "spectator")))
	{
		i = atoi(c);
		if (i)
		{
			client->spectator = true;
		}
	}

	memset(&server->challenges[x], 0, sizeof(struct challenge));
	p = (char *)Packet_Create(&p_size, "ic", HEADER_ID, S2C_CONNECTION);
	NET_Send(server->net, p, p_size, &client->address);
	free(p);
	Log_Print(server->log, log_debug, "added \"%s\" in slot: %i", client->name, i);
	Log_Print(server->log, log_main, "\"%s\" joined.", client->name);
	client->state = cs_connected;
}

static qboolean Server_WriteDelta(struct server *server, struct client *client, struct entity_state *from, struct entity_state *to, qboolean force)
{
	int bits, i;

	bits = 0;

	for (i=0; i<3; i++)
		if (to->origin[i] != from->origin[i])
			bits |= U_ORIGIN1<<i;

	if (to->angles[0] != from->angles[0])
		bits |= U_ANGLE1;

	if (to->angles[1] != from->angles[1])
		bits |= U_ANGLE2;

	if (to->angles[2] != from->angles[2])
		bits |= U_ANGLE3;

	if (to->colormap != from->colormap)
		bits |= U_COLORMAP;

	if (to->skinnum != from->skinnum)
		bits |= U_SKIN;

	if (to->frame != from->frame)
		bits |= U_FRAME;

	if (to->effects != from->effects)
		bits |= U_EFFECTS;

	if (to->model_index != from->model_index)
		bits |= U_MODEL;

	if (bits & U_CHECKMOREBITS)
		bits |= U_MOREBITS;

	if (to->flags & U_SOLID)
		bits |= U_SOLID;

	if (!to->number)
		return false;

	if (!bits && !force)
		return true;

	i = to->number | (bits&~U_CHECKMOREBITS);
	if (i &U_REMOVE)
	{
#warning make the server end here
		return false;
	}

	Client_Write(client, "S", i);

	if (bits & U_MOREBITS)
		Client_Write(client, "c", bits&255);
	if (bits & U_MODEL)
		Client_Write(client, "c", to->model_index);
	if (bits & U_FRAME)
		Client_Write(client, "c", to->frame);
	if (bits & U_COLORMAP)
		Client_Write(client, "c", to->colormap);
	if (bits & U_SKIN)
		Client_Write(client, "c", to->skinnum);
	if (bits & U_EFFECTS)
		Client_Write(client, "c", to->effects);
	if (bits & U_ORIGIN1)
		Client_Write(client, "C", to->origin[0]);
	if (bits & U_ANGLE1)
	{
		printf("angle0\n");
		Client_Write(client, "A", to->angles[0]);
	}
	if (bits & U_ORIGIN2)
		Client_Write(client, "C", to->origin[1]);
	if (bits & U_ANGLE2)
	{
		printf("angle1\n");
		Client_Write(client, "A", to->angles[1]);
	}
	if (bits & U_ORIGIN3)
		Client_Write(client, "C", to->origin[2]);
	if (bits & U_ANGLE3)
	{
		printf("angle2\n");
		Client_Write(client, "A", to->angles[2]);
	}

	return true;
}

static void Server_WriteUpdate(struct server *server, struct client *client, struct packet_entities *to)
{
	struct edict *e;
	struct frame *frame, *frame_from;
	struct packet_entities *from;
	int oldmax, newmax, oldindex, newindex, oldnum, newnum;
	int bits;

	frame = &client->frames[client->incoming_sequence & UPDATE_MASK];

	if (0)
	{
		printf("frame seq: %i\n", client->incoming_sequence & UPDATE_MASK);
		printf("client delta: %i\n", client->delta_sequence);
	}

	if (client->delta_sequence != -1)
	{
		frame_from = &client->frames[client->delta_sequence & UPDATE_MASK];
		from = &frame_from->entities;
		oldmax = from->entities_count;
		Client_Write(client, "cc", svc_deltapacketentities, client->delta_sequence);
	}
	else
	{
		oldmax = 0;
		from = NULL;
		Client_Write(client, "c", svc_packetentities);
	}

	newindex = 0;
	oldindex = 0;

	while (newindex < to->entities_count || oldindex < oldmax)
	{
		newnum = newindex >= to->entities_count ? 9999 : to->entities[newindex].number;
		oldnum = oldindex >= oldmax ? 9999 : from->entities[oldindex].number;

		if (newnum == oldnum)
		{
			Server_WriteDelta(server, client, &from->entities[oldindex], &to->entities[newindex], false);
			newindex++;
			oldindex++;
			continue;
		}

		if (newnum < oldnum)
		{
			e = &server->edicts[newnum];
			Server_WriteDelta(server, client, &e->baseline, &to->entities[newindex], true);
			newindex++;
			continue;
		}

		if (newnum > oldnum)
		{
			Client_Write(client, "S", oldnum | U_REMOVE);
			oldindex++;
			continue;
		}
	}

	Client_Write(client, "S", 0);

}

static void Server_WritePlayersToClient(struct server *server , struct client *client)
{
	int i, j;
#warning do this properly

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (server->clients[i].inuse == false)
			continue;


		Client_Write(client, "ccS", svc_playerinfo, i, 0);

		for (j=0; j<3; j++)
			Client_Write(client, "C", server->edicts[i+1].state.origin[j]);

		Client_Write(client, "c", 0);
	}
}

static void Server_WriteStatsToClient(struct server *server, struct client *client)
{
	int i, stats[MAX_CL_STATS];

	for (i=0; i<MAX_CL_STATS;i++)
	{
#warning fix this
		stats[i] = 10;
		if (stats[i] == client->stats[i])
			continue;

		client->stats[i] = stats[i];
		if (stats[i] >= 0 && stats[i] <= 255)
			Client_WriteReliable(client, "bbb", svc_updatestat, i, stats[i]);
		else
			Client_WriteReliable(client, "bbi", svc_updatestat, i, stats[i]);
	}
}

static void Server_WriteEntitiesToClient(struct server *server, struct client *client)
{
	struct packet_entities *to;
	struct pvs pvs;
	struct edict *e;
	int i;
	vec3_t origin;
	struct frame *frame;

	frame = &client->frames[client->incoming_sequence & UPDATE_MASK];
	to = &frame->entities;
	to->entities_count = 0;

	e = NULL;

	Server_WritePlayersToClient(server, client);

	for (i=1+MAX_CLIENTS; i < server->edicts_count; i++, e = &server->edicts[i])
	{
		if (to->entities_count == MAX_PACKET_ENTITIES)
			continue;

		if (e == NULL)
			continue;

		if (e->inuse == false)
			continue;

		memcpy(&to->entities[to->entities_count], &e->state, sizeof(struct entity_state));
		to->entities[to->entities_count].number = i;
		to->entities[to->entities_count].flags = 0;
		to->entities_count++;
	}
	Server_WriteUpdate(server, client, to);
}

static void Server_WriteClientsToClient(struct server *server, struct client *client)
{
	int i, j, pflags;
	struct client *c;
	struct entity_state *e;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (!server->clients[i].inuse)
			continue;

		if (!server->clients[i].state != cs_spawned)
			continue;

		c = &server->clients[i];
		e = &client->edict->state;

		pflags = PF_MSEC | PF_COMMAND;

		if (e->model_index != server->player_model)
			pflags |= PF_MODEL;

		for (j=0; j<3; j++)
			if (e->velocity[i])
				pflags |= PF_VELOCITY1<<j;

		if (e->effects)
			pflags |= PF_EFFECTS;

		if (e->skin)
			pflags |= PF_SKINNUM;

		if (e->health <= 0)
			pflags |= PF_DEAD;
	}
}

/*
 * creates a full client update message in a buffer
 */
static qboolean Server_CreateFullClientUpdate(struct server *server, struct client *client, struct buffer *buffer)
{
	int i;
	struct buffer info;

	if (!buffer || !client || !buffer)
		return false;

	i = client - server->clients;
	info.position = 0;

	if (!Info_CopyToBuffer(client->userinfo, &info, IF_CLIENT))
		return false;

	if (info.position == 0)
		return false;

	Packet_WriteToBuffer(buffer, "ccS", svc_updatefrags, i, 0);
	Packet_WriteToBuffer(buffer, "ccS", svc_updateping, i, 13);
	Packet_WriteToBuffer(buffer, "ccc", svc_updatepl, i, 0);
	Packet_WriteToBuffer(buffer, "ccf", svc_updateentertime, i, 3.0f);
	Packet_WriteToBuffer(buffer, "ccis", svc_updateuserinfo, i, client->id, info.data);

	return true;
}

/*
 * sends a full client update to every client
 */
void Server_FullClientUpdate(struct server *server, struct client *client)
{
	int i;
	struct buffer update;

	update.position = 0;
	if (!Server_CreateFullClientUpdate(server, client, &update))
		return;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (!server->clients[i].inuse)
			continue;

		Client_WriteReliable(&server->clients[i], "B", &update);
	}
}

/*
 * sends a full client update to a specific client
 */
void Server_FullClientUpdateToClient(struct server *server, struct client *client)
{
	int i;
	struct buffer update;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (!server->clients[i].inuse)
			continue;

		update.position = 0;

		if (!Server_CreateFullClientUpdate(server, &server->clients[i], &update))
			continue;

		Client_WriteReliable(client, "B", &update);
	}
}

static void Server_SendPacket(struct server *server, struct client *client)
{
	struct client *c;
	int os, is;
	qboolean send_reliable;// = false;
	struct buffer packet;

	c = client;

	// if the remote site dropped the last reliable message, resend
	send_reliable = false;

	if (	c->incoming_acknowledged > c->last_reliable_sequence
		&&	c->incoming_reliable_acknowledged != c->reliable_sequence)
		send_reliable = true;

	// if the reliable buffer is empty copy the current buffer
	if (c->reliable.position == 0 && c->message.position)
	{
		memcpy(c->reliable.data, c->message.data, c->message.position);
		c->reliable.position = c->message.position;
		c->message.position = 0;
		c->reliable_sequence ^= 1;
		send_reliable = true;
	}

	if (print_packets)
		printf("%i %i %i %i\n",
			c->incoming_acknowledged, c->last_reliable_sequence,
			c->incoming_reliable_acknowledged, c->reliable_sequence);
	if (print_packets)
		printf("server     o=%i(%i) i=%i(%i) %i %i\n",
			c->outgoing_sequence, (send_reliable << 31),
			c->incoming_sequence, c->incoming_reliable_sequence,
			send_reliable, c->reliable.position);
	os = c->outgoing_sequence | (send_reliable << 31);
	is = c->incoming_sequence | (c->incoming_reliable_sequence << 31);

	c->outgoing_sequence++;
	memset(&packet, 0, sizeof(struct buffer));
	// write the header
	if (!Packet_WriteToBuffer(&packet, "ii", os, is))
	{
		Print_Console("error writing packet.\n");
		return;
	}

	Server_WriteClientsToClient(server, client);

	if (send_reliable)
	{
		if(!Buffer_Copy(&packet, &c->reliable, true))
		{
			Print_Console("this shouldnt happen\n");
			return;
		}
		c->last_reliable_sequence = c->outgoing_sequence;
	}

	if (c->state == cs_spawned)
	{
		Server_WriteEntitiesToClient(server, c);
		if (Buffer_Copy(&packet, &c->message, true))
			c->message.position = 0;
		Server_WriteStatsToClient(server, c);
	}

	//Buffer_Copy(&server->datagram, &packet, true);

	if (print_packets)
		printf("sending packet: %i\n", packet.position);
	if (0)
		for (is=0;is<packet.position;is++)
			printf("%3i: %c\n", is, packet.data[is]);

	NET_Send(server->net, &packet.data, packet.position, &c->address);
}

static void CLC_Move(struct server *server, struct client *client, struct packet *packet)
{
	int checksum_offset;
	int checksum;
	int lossage;

	checksum_offset = packet->offset;
	checksum = Packet_ReadByte(packet);
	lossage = Packet_ReadByte(packet);

	memset(&client->ucmd_oldest, 0, sizeof(struct usercmd));
	Packet_ReadDeltaUsercmd(packet, &client->ucmd_oldest);
	memcpy(&client->ucmd_old, &client->ucmd_oldest, sizeof(struct usercmd));
	Packet_ReadDeltaUsercmd(packet, &client->ucmd_old);
	memcpy(&client->ucmd_new, &client->ucmd_old, sizeof(struct usercmd));
	Packet_ReadDeltaUsercmd(packet, &client->ucmd_new);
}

static void CLC_StringCmd(struct server *server, struct client *client, struct packet *packet)
{
	struct tokenized_string *ts;
	char *string;
	int slen;

	string = Packet_ReadString(packet);
	slen = strlen(string);
	//remove \n
	if (slen > 0)
		if (string[slen-1] == '\n')
			string[slen-1] = '\0';
	Log_Print(server->log, log_debug, "StringCmd: from %s\n\t%s", client->name, string);
	ts = Tokenize_String(string);
	if (ts)
	{

		if (!LUA_CallFunctionTokens(server, &server->mod, client, ts->tokens[0], ts, 1))
		{
			if (String_Compare(ts->tokens[0], "say"))
				CMD_Say(server, client, ts);
			else if (String_Compare(ts->tokens[0], "new"))
				CMD_New(server, client, ts);
			else if (String_Compare(ts->tokens[0], "soundlist"))
				CMD_Soundlist(server, client, ts);
			else if (String_Compare(ts->tokens[0], "modellist"))
				CMD_Modellist(server, client, ts);
			else if (String_Compare(ts->tokens[0], "prespawn"))
				CMD_Prespawn(server, client, ts);
			else if (String_Compare(ts->tokens[0], "spawn"))
				CMD_Spawn(server, client, ts);
			else if (String_Compare(ts->tokens[0], "begin"))
				CMD_Begin(server, client, ts);
			else if (String_Compare(ts->tokens[0], "drop"))
				CMD_Drop(server, client, ts);
			else if (String_Compare(ts->tokens[0], "setinfo"))
				CMD_Setinfo(server, client, ts);
			else if (String_Compare(ts->tokens[0], "download"))
				CMD_Download(server, client, ts);
		}

		Tokenize_String_Delete(ts);
	}
}

static void Server_ProcessMessage(struct server *server ,struct packet *packet, int clientid)
{
	unsigned int i;
	unsigned int sequence_ack, sequence;
	unsigned int reliable_ack, reliable_message;
	struct client *client;

	client = &server->clients[clientid];

	if (print_packets)
		printf("client(%i): ", clientid);
	i = packet->data.data.sequence.c[0]
		+ (packet->data.data.sequence.c[1] << 8)
		+ (packet->data.data.sequence.c[2] << 16)
		+ (packet->data.data.sequence.c[3] << 24);
	sequence = i;

	i = packet->data.data.sequence_ack.c[0]
		+ (packet->data.data.sequence_ack.c[1] << 8)
		+ (packet->data.data.sequence_ack.c[2] << 16)
		+ (packet->data.data.sequence_ack.c[3] << 24);
	sequence_ack = i;

	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;

	sequence &= ~(1<<31);
	sequence_ack &= ~(1<<31);

	if (client->backbuffer_count)
		if(Buffer_Copy(&client->message, client->reliable_buffer, true))
			Client_RemoveBackbuffer(client);

	if (sequence < client->incoming_sequence)
	{
		printf("this happens! -> %s\n", client->name);
		return;
	}

	if (print_packets)
		printf("s=%i(%i) a=%i(%i)[%i] %i\n", sequence, reliable_message, sequence_ack, reliable_ack, (unsigned)client->reliable_sequence, packet->data.data.port);
	// reliable message has been recieved
	if (reliable_ack == (unsigned)client->reliable_sequence)
	{
		client->reliable.position = 0;
		if (print_packets)
			printf("client got the reliable packet\n");
	}

	client->incoming_sequence = sequence;
	client->incoming_acknowledged = sequence_ack;
	client->incoming_reliable_acknowledged = reliable_ack;
	if (reliable_message)
	{
		client->incoming_reliable_sequence ^= 1;
	}
//	Server_SendPacket(server, clientid);

	if (client->incoming_sequence >= client->outgoing_sequence)
	{
		client->outgoing_sequence = client->incoming_sequence;
		client->packet_recieved = true;
	}
	else
	{
		client->packet_recieved = false;
	}

	client->delta_sequence = -1;
	//handle message
	while(Packet_ReadMessageType(packet))
	{
		if (packet->message_type == -1)
		{
			break;
		}

		switch (packet->message_type)
		{
			case clc_nop:
				break;
			case clc_move:
				CLC_Move(server, client, packet);
				break;
			case clc_stringcmd:
				CLC_StringCmd(server, client, packet);
				break;
			case clc_delta:
				client->delta_sequence = Packet_ReadByte(packet);
				break;
			default:
				printf("unknown message: %i\n", packet->message_type);
				break;
		}
	}
}

static void Server_HandlePacket(struct server *server, struct packet *packet)
{
	struct tokenized_string *ts;
	int i;

	if (packet->data.oob_data.header == HEADER_ID)
	{
		packet->length_with_header = packet->length;
		packet->length = packet->length - 4;
#warning fix this somewhere properly
		//remove the trailing \n
		if (packet->data.oob_data.data[packet->length - 1] == '\n')
			packet->data.oob_data.data[packet->length- 1] = '\0';


		ts = Tokenize_String(packet->data.oob_data.data);

		if (ts->count > 0)
		{
			if (String_Compare(ts->tokens[0], "getchallenge"))
			{
				Log_Print(server->log, log_debug, "its a challenge packet");
				Server_HandleChallenge(server, packet);
			}
			else if (String_Compare(ts->tokens[0], "connect"))
			{
				Log_Print(server->log, log_debug, "its a connect packet");
				Server_HandleConnect(server, packet, ts);
			}
			else
			{
				Log_Print(server->log, log_debug, "unknown packet: %s - %s", ts->tokens[0], (char *)&packet->data);
			}
		}
		Tokenize_String_Delete(ts);
		return;
	}
	else
	{
		packet->length_with_header = packet->length;
		packet->length = packet->length - 10;
		//find out from wich client
		for (i=0;i<MAX_CLIENTS;i++)
		{
			if (server->clients[i].inuse == false)
				continue;
			if (NET_CompareAdr(&server->clients[i].address, &packet->address))
				break;
		}

		if (i == MAX_CLIENTS)
		{
			if (print_packets)
				Print_Console("a packet came in wich shouldnt. (%i)\n", packet->length);
			return;
		}

		Server_ProcessMessage(server, packet, i);
		return;
	}
}

static void Server_LoadEntities(struct server *server, char *entity_string)
{
	char *s, *c;
	int x;

	s = entity_string;
	while (*s)
	{
		s = strchr(s, '{');
		if (s == NULL)
			break;
		if (*s == '{')
			s++;
		if (*s == '\n')
			s++;
		c = strchr(s, '}');
		if (c == NULL)
			break;
		if (*c == '}')
			c--;
		if (*c == '\n')
			c--;
		x = c - s + 1;
		LUA_CallFunctionArguments(server, &server->mod, "entity_load", 0, false, "S", s, x);
		s = c;
	}
	LUA_CallFunction(server, &server->mod, NULL, "print_info");
}

/*
 * create the signon message and at the same time create the baseline for the statics
 */
static qboolean Server_CreateSignon(struct server *server)
{
	int i, x;
	struct edict *e;
	struct buffer *buffer;

	Log_Print(server->log, log_debug, "Server_CreateSignon edicts: %i", server->edicts_count);

	for (i=1; i<MAX_EDICTS; i++)
	{
		e = &server->edicts[i];
		if (!e->inuse)
			continue;

		if (i>0 && i<MAX_CLIENTS)
		{
			e->state.colormap = i;
		}
		else
		{
			e->state.colormap = 0;
		}

		if (server->signon_buffers[server->signon_buffers_count].position > BUFFER_SIZE - 512)
		{
			server->signon_buffers_count++;
		}
		if (server->signon_buffers_count >= MAX_SIGNON_BUFFERS)
		{
			Log_Print(server->log, log_debug, " Server_CreateSignon: real problem...");
			return false;
		}

		memcpy(&e->baseline, &e->state, sizeof(struct entity_state));

		buffer = &server->signon_buffers[server->signon_buffers_count];

		Packet_WriteToBuffer(buffer, "cS", svc_spawnbaseline, i);
		Packet_WriteToBuffer(buffer, "c", e->state.model_index);
		Packet_WriteToBuffer(buffer, "c", e->state.frame);
		Packet_WriteToBuffer(buffer, "c", e->state.colormap);
		Packet_WriteToBuffer(buffer, "c", e->state.skinnum);

		//printf("%i: %i - %s %i\n", i, e->state.model_index, server->model_precache[e->state.model_index], e->state.skinnum);
		for (x=0; x<3; x++)
		{
			//printf("%4.4f - %4.4f\n", e->state.origin[x], e->state.angles[x]);
			Packet_WriteToBuffer(buffer, "C", e->state.origin[x]);
			Packet_WriteToBuffer(buffer, "A", e->state.angles[x]);
		}
	}
	return true;
}

static qboolean Server_LoadMap(struct server *server)
{
	int i;
	char buffer[64];
	struct edict *e;

	server->spawn_count++;

	// resetting everything
	//models
	for (i=0;i<MAX_MODELS;i++)
	{
		free(server->model_precache[i]);
		server->model_precache[i] = NULL;
	}

	//sounds
	for (i=0;i<MAX_SOUNDS;i++)
	{
		free(server->sound_precache[i]);
		server->sound_precache[i] = NULL;
	}


	//precache the map
	snprintf(buffer, sizeof(buffer), "maps/%s.bsp", server->map_filename);
	Log_Print(server->log, log_debug, "map precache: %i", Server_PrecacheModel(server, buffer, true));

	//load it actually
	Model_MapFree(server->map);
	server->map = Model_MapLoad(server, buffer);

	if (!server->map)
	{
		Log_Print(server->log, log_main, "ERROR: loading map \"%s\" server will shutdown", buffer);
		return false;
	}

	if (!World_SetupAreaNodes(server))
		return false;

	// reserve the first MAX_CLIENTS+1 edicts
	e = Server_GetFreeEdict(server);
	if (!e)
		return false;
	e->state.model_index = Server_PrecacheModel(server, buffer, true);
	e->solid = SOLID_BSP;

	for (i=0; i<MAX_CLIENTS; i++)
	{
		e = Server_GetFreeEdict(server);
		if (!e)
			return false;
		server->clients[i].edict = e;
		e->inuse = true;
	}

	for (i=1; i<server->map->submodels_count; i++)
	{
		snprintf(buffer, sizeof(buffer), "*%i", i);
		e = Server_GetFreeEdict(server);
		if (!e)
			return false;
		e->state.model_index = Server_PrecacheModel(server, buffer, true);
	}

	//load entities
	LUA_CallFunction(server, &server->mod, NULL, "entity_preload");
	Server_LoadEntities(server, server->map->entity_string);
	//Log_Print(server->log, log_debug, "%s", server->map->entity_string);
	LUA_CallFunction(server, &server->mod, NULL, "entity_load_finished");

	for (i=0; i<32; i++)
		LUA_GetSpawn(server, &server->edicts[i+1].state.origin);

	if (!Server_CreateSignon(server))
		return false;

	//call the mod map function
	LUA_CallFunction(server, &server->mod, NULL, "map_start");

	for (i=0; i<MAX_CLIENTS; i++)
		server->player_model = server->clients[i].edict->state.model_index = Server_PrecacheModel(server, "progs/player.mdl", true);

	if (0)
	{
	for (i=0; ;i++)
	{
		if (server->edicts[i].inuse == false)
			break;

		printf("%i: %i %s - %f %f %f - %f %f %f\n", i, server->edicts[i].state.model_index,
				server->model_precache[server->edicts[i].state.model_index],
				server->edicts[i].state.origin[0],
				server->edicts[i].state.origin[1],
				server->edicts[i].state.origin[2],
				server->edicts[i].state.angles[0],
				server->edicts[i].state.angles[1],
				server->edicts[i].state.angles[2]
				);
	}

	for (i=0; 0 && i<server->map->submodels_count; i++)
	{
		printf("%i: %f %f %f\n", i ,
				server->map->submodels[i].origins[0],
				server->map->submodels[i].origins[1],
				server->map->submodels[i].origins[2]);
	}
	}

	return true;
}

static void Server_ChangeMap(struct server *server, char *mapname)
{
	Log_Print(server->log, log_main, "loading map \"%s\"", mapname);
	server->run = false;
	if (server->map_filename)
		free(server->map_filename);

	server->map_filename = strdup(mapname);
	if(Server_LoadMap(server) == false)
		return;

	server->run = true;
}

static void Server_Frame(struct server *server)
{
	int r;
	struct packet p;
	int i;

	memset(&p, 0, sizeof(p));
	if (server->frametime == 0)
		server->frametime = 0.001;
	server->frametime = server->realtime;
	server->realtime = Tools_DoubleTime(server);
	server->frametime = server->realtime - server->frametime;
	time(&server->time_current);

	LUA_Server_SetVariable(server, &server->mod, "dd", "realtime", server->realtime, "frametime", server->frametime);

	while((r = NET_Recieve(server->net, &p.data, 4096, &p.address)) > 0)
	{
		p.offset = 0;
		p.length = r;
		Server_HandlePacket(server, &p);
	}

	Physics_Frame(server);

	for (i=0; i<MAX_CLIENTS; i++)
	{
		if (server->clients[i].inuse && server->clients[i].packet_recieved)
		{
//			printf("%f %f %f\n"	, server->edicts[i+1].state.origin[0]
//								, server->edicts[i+1].state.origin[1]
//								, server->edicts[i+1].state.origin[2]);

			Server_SendPacket(server , &server->clients[i]);
			server->clients[i].packet_recieved = false;
		}
	}

	//run at around 72 fps atm
	usleep(13000);
}

static void Server_Quit(struct server *server)
{
}

int main (int argc, char *argv[])
{
	struct server *server;
	int i;
	struct plane *p;

//	signal(SIGINT, Server_SignalHandler);
	signal(SIGQUIT, Server_SignalHandler);

	server = calloc(1, sizeof(*server));
	if (server == NULL)
	{
		Print_Console("wow...\n");
		return 1;
	}

	server->pid = getpid();
	time(&server->time_start);

//	server->debug_lua_stack = true;

	server_handles.server[0] = server;
	server_handles.server[1] = NULL;

	server->port = 27500;

	if (Server_HandleArguments(server, argc, argv))
	{
#ifndef __FIX_THIS_WEIRD_BUG__
		if (server->disable_log == false)
			if (!(server->log = Log_Init(server)))
				Print_Console("could not init logging...\n");
#endif
		if (Server_DefaultArguments(server))
		{
			if(LUA_Init(server))
			{
				if(NET_Init(server))
				{
					Server_ChangeMap(server, "aerowalk");
					Log_Print(server->log, log_main, "Starting Server on: %s:%i\n", server->ip? server->ip : "any", server->port);
					server->run = true;
					while (server->run)
						Server_Frame(server);
					Server_Quit(server);
					Server_Cleanup(server);
					return 0;
				}
			}
		}
	}
	Server_Cleanup(server);
	return 1;
}

int Server_PrecacheSound(struct server *server, char *sound)
{
	int i;
	if (!server || !sound)
		return -1;

	if (server->sound_precache_index >= MAX_SOUNDS)
		return -1;

	i = server->sound_precache_index;

	server->sound_precache[i] = strdup(sound);
	if (server->sound_precache[i] == NULL)
		return -1;

	Log_Print(server->log, log_debug, "precaching sound: %s %i", sound, i);
	server->sound_precache_index++;

	return i;
}

int Server_PrecacheModel(struct server *server, char *model, qboolean add)
{
	int i;
	if (!server || !model)
		return -1;

	if (server->model_precache_index >= MAX_MODELS)
		return -1;

	// check if its precached
	for (i=0; i<server->model_precache_index; i++)
		if (strcmp(server->model_precache[i], model) == 0)
			return i;

	if (!add)
		return -1;

	i = server->model_precache_index;

	server->model_precache[i] = strdup(model);
	if (server->model_precache[i] == NULL)
		return -1;

	Log_Print(server->log, log_debug, "precaching model: %s %i", model, i);
	server->model_precache_index++;

	return i;
}

int Server_PrecacheModelNet(struct server *server, char *model, qboolean add)
{
	return Server_PrecacheModel(server, model, add) + 1;
}

int Server_AddLightstyle(struct server *server, char *style)
{
	int i;
	if (!server || !style)
		return -1;

	if (server->lightstyles_index >= MAX_LIGHTSTYLES)
		return -1;
	for (i=0; i<server->lightstyles_index; i++)
		if (strcmp(server->lightstyles[i], style) == 0)
			return i;

	i = server->lightstyles_index;

	server->lightstyles[i] = strdup(style);
	if (server->lightstyles[i] == NULL)
		return -1;

	server->lightstyles_index++;
	return i;
}

qboolean Server_ClientChangeName(struct server *server, struct client *client, char *newname)
{
	Print_ToAll(server, false, 3, "%s changed name to %s\n", client->name, newname);
	snprintf(client->name, sizeof(client->name), "%s", newname);
	return true;
}

struct edict *Server_GetEdictForInlineModel(struct server *server, char *model)
{
	int m;
	if (!server | !model)
		return NULL;

	if (model[0] != '*')
		return NULL;

	m = atoi(model+1);

	m++;

	return &server->edicts[MAX_CLIENTS + m];
}

struct hull *boxhull_allocate(void)
{
	struct hull *hull;
	int i, side;

	hull = calloc(1, sizeof(*hull));

	if (hull)
	{
		hull->planes = calloc(6, sizeof(struct plane));
		if (hull->planes)
		{
			hull->clipnodes = calloc(6, sizeof(struct clipnode));
			if (hull->clipnodes)
			{
				hull->firstclipnode = 0;
				hull->lastclipnode = 5;
				for (i=0; i<6; i++)
				{
					side = i&1;
					hull->clipnodes[i].children[side] = CONTENTS_EMPTY;
					hull->clipnodes[i].children[side^1] = (i != 5) ? (i + 1) : CONTENTS_SOLID;
					hull->planes[i].type = i>>1;
					hull->planes[i].normal[i>>1] = 1;
				}
				return hull;
			}
			free(hull->planes);
		}
		free(hull);
	}
	return NULL;
}

static void boxhull_setup(struct hull *hull, vec3_t mins, vec3_t maxs)
{
	if (!hull)
		return;

	hull->planes[0].dist = maxs[0];
	hull->planes[1].dist = mins[0];
	hull->planes[2].dist = maxs[1];
	hull->planes[3].dist = mins[1];
	hull->planes[4].dist = maxs[2];
	hull->planes[5].dist = mins[2];
}

static void boxhull_free(struct hull *hull)
{
	if (!hull)
		return;

	free(hull->clipnodes);
	free(hull->planes);
	free(hull);
}

struct hull *Server_HullForEdict(struct server *server, struct edict *edict, vec3_t mins, vec3_t maxs, vec3_t offset)
{
	vec3_t size, hullmins, hullmaxs;
	int i;
	struct hull *hull;

	if (!server || !edict)
		return NULL;

	if (edict->solid == SOLID_BSP)
	{
		// this is only for map and submodels (i guess)
#warning do movetype push check here
		i = edict->state.model_index;

		Vector_Subtract(size, maxs, mins);
		if (size[0] <3)
			hull = &server->map->submodels[i].hulls[0];
		else if (size[0] <= 32)
			hull = &server->map->submodels[i].hulls[1];
		else
			hull = &server->map->submodels[i].hulls[2];

		// calculate offset
		Vector_Subtract(offset, hull->clip_mins, mins);
		Vector_Add(offset, offset, edict->state.origin);
	}
	else
	{
		// build a hull
		if (edict->hull)
			hull = edict->hull;
		else
		{
			hull = edict->hull = boxhull_allocate();
			if (hull == NULL)
				return NULL;
			edict->hullisallocated = true;
		}

		Vector_Subtract(hullmins, mins, maxs);
		Vector_Subtract(hullmaxs, maxs, mins);

		boxhull_setup(hull, hullmins, hullmaxs);

		Vector_Copy(offset, edict->state.origin);
	}

	return hull;
}
