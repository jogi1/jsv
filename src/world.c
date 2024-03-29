#include <string.h>

#include "server.h"

struct areanode *World_CreateAreaNodes(struct server *server, int depth, vec3_t mins, vec3_t maxs)
{
	struct areanode *anode;
	vec3_t size;
	vec3_t mins1, maxs1, mins2, maxs2;

	anode = &server->areanodes[server->areanodes_count];
	server->areanodes_count++;

#warning clear links here
	if (depth == AREA_DEPTH)
	{
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}

	Vector_Subtract(size, maxs, mins);
	if (size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = -1;

	anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
	Vector_Copy(mins1, mins);
	Vector_Copy(mins2, mins);
	Vector_Copy(maxs1, maxs);
	Vector_Copy(maxs2, maxs);

	maxs1[anode->axis] = mins1[anode->axis] = anode->dist;

	anode->children[0] = World_CreateAreaNodes(server, depth+1, mins2, maxs2);
	anode->children[1] = World_CreateAreaNodes(server, depth+1, mins1, maxs1);

	return anode;
}

qboolean World_SetupAreaNodes(struct server *server)
{
	server->areanodes_count = 0;
	memset(server->areanodes, 0, sizeof(struct areanode) * AREA_NODES);

	World_CreateAreaNodes(server, 0, server->map->submodels[0].mins, server->map->submodels[0].maxs);
	return true;
}

int World_AreaEdicts(struct server *server, vec3_t mins, vec3_t maxs, struct edict **edicts, int max_edicts, int area)
{
	struct link *start, *l;
	struct edict *touch;
	int count = 0;
	int stackdepth = 0;
	struct areanode *localstack[AREA_NODES], *node = server->areanodes;

	while(1)
	{
		if (area == AREA_SOLID)
			start = &node->solid_edicts;
		else
			start = &node->trigger_edicts;

		for (l = start->next; l != start; l = l->next)
		{
			touch = l->e;
			if (touch->solid == SOLID_NOT)
				continue;

			if (	mins[0] > touch->absmax[0]
				||	mins[1] > touch->absmax[1]
				||	mins[2] > touch->absmax[2]
				||	maxs[0] < touch->absmin[0]
				||	maxs[1] < touch->absmin[1]
				||	maxs[2] < touch->absmin[2])
				continue;

			if (count == max_edicts)
				return count;

			edicts[count++] = touch;

			if (node->axis == -1)
				goto checkstack;

			if (maxs[node->axis] > node->dist)
			{
				if (mins[node->axis] < node->dist)
				{
					localstack[stackdepth++] = node->children[0];
					node = node->children[1];
					continue;
				}
				node = node->children[0];
				continue;
			}

			if (mins[node->axis] < node->dist)
			{
				node = node->children[1];
				continue;
			}
		}
checkstack:
		if (!stackdepth)
			return count;
		node = localstack[stackdepth--];
	}
	return count;
}

