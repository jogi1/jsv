#ifndef NET_SRUCTS_H
#define NET_SRUCTS_H

struct net
{
};

enum netaddrtype
{
	NA_LOOPBACK,  /* Only used internally */
	NA_IPV4,
	NA_IPV6,
	NA_NUMTYPES   /* Not a real type */
};

struct netaddr_ipv4
{
	unsigned char address[4];
	unsigned short port;
};

struct netaddr_ipv6
{
	unsigned char address[16];
	unsigned short port;
};

struct netaddr
{
	enum netaddrtype type;

	union
	{
		struct netaddr_ipv4 ipv4;
		struct netaddr_ipv6 ipv6;
	} addr;
};

struct oob_packet_data
{
	unsigned int header;
	char data[4096];
};

struct packet_data
{
	union
	{
		unsigned char c[4];
		unsigned int i;
	} sequence;
	union
	{
		unsigned char c[4];
		unsigned int i;
	} sequence_ack;
	unsigned short port;
	unsigned char data[4096];
};

struct packet
{
	qboolean error;
	struct netaddr address;
	unsigned int length_with_header;
	unsigned int length;
	int offset;
	int message_type;
	union
	{
		struct oob_packet_data oob_data;
		struct packet_data data;
	} data;
};

#endif
