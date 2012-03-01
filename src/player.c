
#include "server.h"

#define MAX_PHYSENTS 64

struct physent
{
	vec3_t origin;
	struct model *model;
	vec3_t mins, maxs;
	int info;
};

struct player_move
{
	vec3_t origin, angles, velocity;
	vec3_t forward, right;
	qboolean jump_held;

	float waterjump_time;
	int time;

	enum pm_type type;

	int physent_count;
	struct physent physents[MAX_PHYSENTS];

	struct usercmd cmd;

	int touch_count;
	int touchindex[MAX_PHYSENTS];
	qboolean on_ground;
	int ground_entity;
	int water_level;
	int water_type;
	double frametime;

	struct edict *edict;
	struct client *client;
};



void Player_Move(struct server *server, struct client *client, struct edict *edict, struct usercmd *cmd)
{
	struct player_move move;

	memset(&move, 0, sizeof(struct player_move));

	move.edict = edict;
	move.client = client;
	move.frametime = cmd->msec * 0.001;

}
