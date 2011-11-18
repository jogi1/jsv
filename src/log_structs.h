#ifndef LOG_S_H
#define LOG_S_H

enum log_type
{
	log_main,
	log_lua,
	log_debug,
	log_player,
	log_player_packet
};

struct log_entry
{
	qboolean done;
	enum log_type;
	int player_num;
	unsigned int type;
	char *string;
	time_t time;
	struct log_entry *next;
};

struct log
{
	qboolean run;
	char *log_dir;
	FILE *main, *lua, *debug, *players;
	struct log_entry *first, *last;
};


#endif
