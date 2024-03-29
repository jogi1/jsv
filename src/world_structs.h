#ifndef WORLD_STRUCTS_H
#define WORLD_STRUCTS_H

#define AREA_SOLID 0
#define AREA_TRIGGER 1
#define AREA_DEPTH 4
#define AREA_NODES 32


struct link
{
	struct edict *e;
	struct link *prev, *next;
};

struct areanode
{
	int axis;
	float dist;
	struct areanode *children[2];
	struct link solid_edicts;
	struct link trigger_edicts;
};

#endif
