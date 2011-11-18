#include "qtypes.h"
#include "net_structs.h"
#include "tools_structs.h"
#include "packet_structs.h"
#include "model_structs.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <time.h>
#include "pthread.h"
#include "lua_structs.h"
#include "log_structs.h"

#define MAX_SOUNDS 256
#define MAX_MODELS 256
#define MAX_MSGLEN 1450

#define	S2C_CHALLENGE		'c'
#define	S2C_CONNECTION		'j'
#define	A2A_PING		'k'	// respond with an A2A_ACK
#define	A2A_ACK			'l'	// general acknowledgement without info
#define	A2A_NACK		'm'	// [+ comment] general failure
#define A2A_ECHO		'e'	// for echoing
#define	A2C_PRINT		'n'	// print a message on client

#define	S2M_HEARTBEAT		'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN		'C'



#define	svc_bad			0
#define	svc_nop			1
#define	svc_disconnect		2
#define	svc_updatestat		3	// [byte] [byte]
//define	svc_version			4	// [long] server version
#define	svc_setview		5	// [short] entity number
#define	svc_sound		6	// <see code>
//define	svc_time			7	// [float] server time
#define	svc_print		8	// [byte] id [string] null terminated string
#define	svc_stufftext		9	// [string] stuffed into client's console buffer
					// the string should be \n terminated
#define	svc_setangle		10	// [angle3] set the view angle to this absolute value

#define	svc_serverdata		11	// [long] protocol ...
#define	svc_lightstyle		12	// [byte] [string]
//define	svc_updatename		13	// [byte] [string]
#define	svc_updatefrags		14	// [byte] [short]
//define	svc_clientdata		15	// <shortbits + data>
#define	svc_stopsound		16	// <see code>
//define	svc_updatecolors	17	// [byte] [byte] [byte]
//define	svc_particle		18	// [vec3] <variable>
#define	svc_damage		19

#define	svc_spawnstatic		20
//	svc_spawnbinary		21
#define	svc_spawnbaseline	22

#define	svc_temp_entity		23	// variable
#define	svc_setpause		24	// [byte] on / off
//	svc_signonnum		25	// [byte]  used for the signon sequence

#define	svc_centerprint		26	// [string] to put in center of the screen

#define	svc_killedmonster	27
#define	svc_foundsecret		28

#define	svc_spawnstaticsound	29	// [coord3] [byte] samp [byte] vol [byte] aten

#define	svc_intermission	30	// [vec3_t] origin [vec3_t] angle
#define	svc_finale		31	// [string] text

#define	svc_cdtrack		32	// [byte] track
#define svc_sellscreen		33

#define	svc_smallkick		34	// set client punchangle to 2
#define	svc_bigkick		35	// set client punchangle to 4

#define	svc_updateping		36	// [byte] [short]
#define	svc_updateentertime	37	// [byte] [float]

#define	svc_updatestatlong	38	// [byte] [long]

#define	svc_muzzleflash		39	// [short] entity

#define	svc_updateuserinfo	40	// [byte] slot [long] uid
					// [string] userinfo

#define	svc_download		41	// [short] size [size bytes]
#define	svc_playerinfo		42	// variable
#define	svc_nails		43	// [byte] num [48 bits] xyzpy 12 12 12 4 8
#define	svc_chokecount		44	// [byte] packets choked
#define	svc_modellist		45	// [strings]
#define	svc_soundlist		46	// [strings]
#define	svc_packetentities	47	// [...]
#define	svc_deltapacketentities	48	// [...]
#define svc_maxspeed		49	// maxspeed change, for prediction
#define svc_entgravity		50	// gravity change, for prediction
#define svc_setinfo		51	// setinfo on a client
#define svc_serverinfo		52	// serverinfo
#define svc_updatepl		53	// [byte] [byte]

#define svc_nails2		54	// for interpolation, stores edict num


#define	CM_ANGLE1 	(1<<0)
#define	CM_ANGLE3 	(1<<1)
#define	CM_FORWARD	(1<<2)
#define	CM_SIDE		(1<<3)
#define	CM_UP		(1<<4)
#define	CM_BUTTONS	(1<<5)
#define	CM_IMPULSE	(1<<6)
#define	CM_ANGLE2 	(1<<7)


#define MAX_CHALLENGES 512
#define MAX_CLIENTS 32

#define MAX_SIGNON_BUFFERS 16

#define MAX_EDICTS	512

#define MAX_LIGHTSTYLES 64

#define MAX_BACKBUFFERS 128

#define Q_rint(x) ((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))

#define	UPDATE_BACKUP	64 // copies of entity_state_t to keep buffered. must be power of two
#define	UPDATE_MASK	(UPDATE_BACKUP-1)

#define	U_ORIGIN1	(1<<9)
#define	U_ORIGIN2	(1<<10)
#define	U_ORIGIN3	(1<<11)
#define	U_ANGLE2	(1<<12)
#define	U_FRAME		(1<<13)
#define	U_REMOVE	(1<<14)	// REMOVE this entity, don't add it
#define	U_MOREBITS	(1<<15)

// if MOREBITS is set, these additional flags are read in next
#define	U_ANGLE1	(1<<0)
#define	U_ANGLE3	(1<<1)
#define	U_MODEL		(1<<2)
#define	U_COLORMAP	(1<<3)
#define	U_SKIN		(1<<4)
#define	U_EFFECTS	(1<<5)
#define	U_SOLID		(1<<6)	// the entity should be solid for prediction

#define	U_CHECKMOREBITS	((1<<9) - 1)


enum client_state
{
	cs_unused,
	cs_preconnect,
	cs_connected,
	cs_spawned
};

enum clc_messages
{
	clc_bad,
	clc_nop,
	clc_doublemove,
	clc_move,
	clc_stringcmd,
	clc_delta,
	clc_tmove,
	clc_upload
};

enum pm_type
{
	pm_normal,
	pm_old_spectator,
	pm_spectator,
	pm_dead,
	pm_fly,
	pm_none,
	pm_lock
};

#define	PF_MSEC		(1<<0)
#define	PF_COMMAND	(1<<1)
#define	PF_VELOCITY1	(1<<2)
#define	PF_VELOCITY2	(1<<3)
#define	PF_VELOCITY3	(1<<4)
#define	PF_MODEL	(1<<5)
#define	PF_SKINNUM	(1<<6)
#define	PF_EFFECTS	(1<<7)
#define	PF_WEAPONFRAME	(1<<8)	// only sent for view player
#define	PF_DEAD		(1<<9)	// don't block movement any more
#define	PF_GIB		(1<<10)	// offset the view height differently
// bits 11..13 are player move type bits (ZQuake extension)
#define PF_PMC_SHIFT	11
#define	PF_PMC_MASK	7
#define	PF_ONGROUND	(1<<14)		// ZQuake extension
#define	PF_SOLID		(1<<15)			// ZQuake extension

// player move types
#define PMC_NORMAL		0	// normal ground movement
#define PMC_NORMAL_JUMP_HELD	1	// normal ground novement + jump_held
#define PMC_OLD_SPECTATOR	2	// fly through walls (QW compatibility mode)
#define PMC_SPECTATOR		3	// fly through walls
#define PMC_FLY			4	// fly, bump into walls
#define PMC_NONE		5	// can't move (client had better lerp the origin...)
#define PMC_LOCK		6	// server controls origin and view angles
#define PMC_EXTRA3		7	// future extension

static char *clc_message_text [] =
{
	"clc_bad",
	"clc_nop",
	"clc_doublemove",
	"clc_move",
	"clc_stringcmd",
	"clc_delta",
	"clc_tmove",
	"clc_upload"
};


struct challenge
{
	double time;
	int challenge;
	struct netaddr address;
};

struct usercmd
{
	unsigned char msec;
	vec3_t	angles;
	short	forwardmove, sidemove, upmove;
	unsigned char buttons;
	unsigned char impulse;
};

#define BUFFER_SIZE 1450

struct buffer
{
	char data[BUFFER_SIZE];
	int position;
};

struct entity_state
{
	int number; //edict index

	int flags;
	vec3_t origin, angles;
	int model_index;
	int frame;
	int colormap;
	int skinnum;
	int effects;

	// server stuff... put this somewhere else?
	vec3_t velocity, mins;
	int skin;
	int health;
};

#define MAX_PACKET_ENTITIES 64
struct packet_entities
{
	int entities_count;
	struct entity_state entities[MAX_PACKET_ENTITIES];
};

struct frame
{
	double senttime;
	float ping_time;

	struct packet_entities entities;
};

#define CLIENT_NAME_MAX 32
#define CLIENT_INFO_MAX 128


struct key_val
{
	char *key, *value;
	int flags;
};

struct info
{
	struct key_val info[CLIENT_INFO_MAX];
	int size;
};

struct client
{
	qboolean inuse;
	qboolean packet_recieved;
	qboolean spectator;
	int id;
	int challenge;
	int qport;
	struct netaddr address;
	enum client_state state;
	unsigned int outgoing_sequence;
	unsigned int incoming_sequence, incoming_acknowledged;
	unsigned int incoming_reliable_sequence, incoming_reliable_acknowledged;
	unsigned int last_reliable_sequence;
	unsigned int reliable_sequence;
	int delta_sequence;
	struct usercmd ucmd_oldest, ucmd_old, ucmd_new;
	struct buffer message;
	struct buffer reliable;						// unacknowledged reliable packet
	struct buffer backbuffer[MAX_BACKBUFFERS];
	struct buffer *reliable_buffer;	//points to either message, if there is space, or a backbuffer
	int backbuffer_count;
	struct buffer packet;
	struct frame frames[UPDATE_BACKUP];

	char *fulluserinfo;
	char name[CLIENT_NAME_MAX];
	struct info *userinfo;

};

enum ed_type
{
	edt_world,
	edt_player,
	edt_static,
	edt_moving
};


#define MAX_ENT_LEAFS 16

struct edict
{
	qboolean	inuse;
	struct entity_state baseline;
	struct entity_state state;
	int numleafs;
	short leafnums[MAX_ENT_LEAFS];
};

struct server
{
	int port;
	int spawn_count;
	qboolean map_change;
	char *map_name;
	qboolean run;
	char *ip;
	char *main_script;
	char *mod_script;
	char *data_dir;
	struct net *net;

	int current_challenge;
	struct challenge challenges[MAX_CHALLENGES];
	struct client clients[MAX_CLIENTS];
	struct buffer datagram;
	int protocol_version;
	struct lua main;
	struct lua mod;

	char *model_precache[MAX_MODELS];
	int model_precache_index;
	char *sound_precache[MAX_SOUNDS];
	int sound_precache_index;
	char *lightstyles[MAX_LIGHTSTYLES];
	int lightstyles_index;

	int player_model;
	int userid;

	struct map *map;
	char *map_filename;

	struct buffer signon_buffers[MAX_SIGNON_BUFFERS];
	int signon_buffers_count;

	struct edict edicts[MAX_EDICTS];
	int edicts_count;

	double realtime;

	unsigned int secbase; //for timing
	time_t time_current;
	time_t time_start;
	struct log *log;
	pthread_t log_thread;
	unsigned int pid;

};


#include "net.h"
#include "packet.h"
#include "tools.h"
#include "tokenize_string.h"
#include "cmd.h"
#include "lua.h"
#include "md4.h"
#include "model.h"
#include "vector.h"
#include "info.h"
#include "log.h"

int Server_PrecacheSound(struct server *server, char *sound);
int Server_PrecacheModel(struct server *server, char *model);
int Server_AddLightstyle(struct server *server, char *style);
struct edict *Server_GetFreeEdict(struct server *server);
void Server_EdictCreateBaseline(struct edict *edict);
qboolean Server_ClientChangeName(struct server *server, struct client *client, char *newname);
void Server_DropClient(struct server *server, struct client *client);
void Server_FullClientUpdateToClient(struct server *server, struct client *client);
