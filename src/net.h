#ifndef NET_H
#define NET_H

#define HEADER_ID 0xFFFFFFFF
#include "qtypes.h"

qboolean NET_Init(struct server *server);
void NET_Shutdown(struct net *net);
int NET_Send(struct net *net, const void *data, int datalen, struct netaddr *address);
int NET_Recieve(struct net *net, void *data, int datalen, struct netaddr *address);
qboolean NET_CompareBaseAdr(const struct netaddr *a, const struct netaddr *b);
qboolean NET_CompareAdr(const struct netaddr *a, const struct netaddr *b);
#endif
