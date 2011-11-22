#include "server.h"

void Physics_Frame(struct server *server)
{
	int i;
	int x, z;
	static int j, k;
	struct client *client;
	vec3_t forward, right, up;

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

		Vector_Angles(server->edicts[i+1].state.angles, forward, right, up);

		Vector_Scale(up, 0.013 * client->ucmd_new.upmove, up);
		Vector_Scale(forward, 0.013 * client->ucmd_new.forwardmove, forward);
		Vector_Scale(right, 0.013 * client->ucmd_new.sidemove, right);

		Vector_Add(server->edicts[i+1].state.origin, server->edicts[i+1].state.origin, up);
		Vector_Add(server->edicts[i+1].state.origin, server->edicts[i+1].state.origin, forward);
		Vector_Add(server->edicts[i+1].state.origin, server->edicts[i+1].state.origin, right);
		for (x=0; x<3; x++)
		{
			server->edicts[i+1].state.angles[x] = client->ucmd_new.angles[x];
		}
	}
}
