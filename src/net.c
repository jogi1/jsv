#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>

#include "server.h"

struct net_internal
{
	int socket;
};

qboolean NET_Init(struct server *server)
{
	struct net_internal *ni;
	int r;
	int one = 1;
	socklen_t addrsize;
	struct sockaddr_in addr;

	if (server == NULL)
		return false;

	ni = calloc(1, sizeof(*ni));
	if (ni)
	{
		ni->socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (ni->socket != -1)
		{
			r = ioctl(ni->socket, FIONBIO, &one);
			if (r == 0)
			{
				memset(&addr, 0, sizeof(struct sockaddr_in));
				addr.sin_family = AF_INET;
				addr.sin_port = htons(server->port);
				*(unsigned int *)&addr.sin_addr.s_addr = 0;
				addrsize = sizeof(addr);

				r = bind(ni->socket, (struct sockaddr *)&addr, addrsize);
				if (r == 0)
				{
					server->net = (struct net *)ni;
					return true;
				}
				else
				Log_Print(server->log, log_debug, "NET_Init: bind error \"%i\"", r);
			}
			else
				Log_Print(server->log, log_debug, "NET_Init: ioctl error \"%i\"", r);
		}
		else
			Log_Print(server->log, log_debug, "NET_Init: socket error");
		free(ni);
	}
	else
		Log_Print(server->log, log_debug, "NET_Init: allocating ni failed");
	return false;
}

void NET_Shutdown(struct net *net)
{
	struct net_internal *ni;

	ni = (struct net_internal *)net;

	if (ni == NULL)
		return;

	close(ni->socket);
	free(ni);
}

int NET_Send(struct net *net, const void *data, int datalen, struct netaddr *address)
{
	struct net_internal *ni;
	struct sockaddr_in addr;

	ni = (struct net_internal *)net;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(address->addr.ipv4.port);
	*(unsigned int *)&addr.sin_addr.s_addr = *(unsigned int *)address->addr.ipv4.address;

	return sendto(ni->socket, data, datalen, 0, (struct sockaddr *)&addr, sizeof(addr));
}

int NET_Recieve(struct net *net, void *data, int datalen, struct netaddr *address)
{
	struct net_internal *ni;
	struct sockaddr_in addr;
	socklen_t fromlen;
	int r;

	ni = (struct net_internal *)net;
	fromlen = sizeof(addr);
	r = recvfrom(ni->socket, data, datalen, 0, (struct sockaddr *)&addr, &fromlen);

	if (fromlen != sizeof(addr))
		return -1;

	address->type = NA_IPV4;
	address->addr.ipv4.port = htons(addr.sin_port);
	*(unsigned int *)address->addr.ipv4.address = *(unsigned int *)&addr.sin_addr.s_addr;

	return r;
}

qboolean NET_CompareBaseAdr(const struct netaddr *a, const struct netaddr *b)
{
	if (a->type != b->type)
		return false;
	
	if (a->type == NA_LOOPBACK)
		return true;
	else if (a->type == NA_IPV4)
	{
		if (*(unsigned int *)a->addr.ipv4.address == *(unsigned int *)b->addr.ipv4.address)
			return true;
	}
	else if (a->type == NA_IPV6)
	{
		if (memcmp(a->addr.ipv6.address, b->addr.ipv6.address, sizeof(a->addr.ipv6.address)) == 0)
			return true;
	}
	return false;
}

qboolean NET_CompareAdr(const struct netaddr *a, const struct netaddr *b)
{
	if (a->type != b->type)
		return false;
	
	if (a->type == NA_LOOPBACK)
		return true;
	else if (a->type == NA_IPV4)
	{
		if (memcmp(&a->addr.ipv4, &b->addr.ipv4, sizeof(a->addr.ipv4)) == 0)
			return true;
	}
	else if (a->type == NA_IPV6)
	{
		if (memcmp(&a->addr.ipv6, &b->addr.ipv6, sizeof(a->addr.ipv6)) == 0)
			return true;
	}
	return false;
}

