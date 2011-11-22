#include "server.h"

void Physics_Frame(struct server *server)
{
	int i;
	int x, z;
	static int j, k;
	struct client *client;

	if (k == 0)
		j++;
	else
		j--;

	if (j < 0)
		k = 0;

	if (j > 77)
		k = 1;


	for (i=0; i<32; i++)
	{
		if (server->clients[i].inuse == false)
			continue;

		client = &server->clients[i];


		server->edicts[i+1].state.origin[0] += 0.013 * client->ucmd_new.forwardmove;
		server->edicts[i+1].state.origin[1] += 0.013 * client->ucmd_new.sidemove;
		server->edicts[i+1].state.origin[2] += 0.013 * client->ucmd_new.upmove;
		for (x=0; x<3; x++)
		{
			server->edicts[i+1].state.angles[x] = client->ucmd_new.angles[x];
		}
	}
}
