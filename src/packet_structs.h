#ifndef PACKETS_STRUCTS_H
#define PACKETS_STRUCTS_H

struct svc_print_s
{
	int type;
	char *string;
};

struct packet_message
{
	qboolean done;
	int type;
	int size;
	union
	{
		struct svc_print_s svc_print_data;
	} data;

	struct packet_message *next;
};

struct packet_message_list
{
	struct packet_message *first, *last;
};

#endif
