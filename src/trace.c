#include <string.h>
#include <stdlib.h>
#include "server.h"

#define DIST_EPSILON 0.03125

struct trace_local
{
	struct hull *hull;
	struct trace *trace;
	int leafs_count;
};

enum
{
	TR_EMPTY, TR_SOLID, TR_BLOCKED
};

int Trace_RecursiveHullTrace(struct trace_local *tl, int num, float p1f, float p2f, const vec3_t p1, const vec3_t p2)
{
	struct plane *plane;
	float t1, t2, frac, midf;
	struct clipnode *node;
	int i, nearside, check, oldcheck;
	vec3_t mid;

	struct hull *hull = tl->hull;
	struct trace *trace = tl->trace;

	/*
	//printf("t_rht: %i - %f - %f\n", num, p1f, p2f);
	*/
	//printf("p1: ");
	//PRINT_VEC(p1);
	//printf("p2: ");
	//PRINT_VEC(p2);

start:
	if (num < 0)
	{
		tl->leafs_count++;
		if (num == CONTENTS_SOLID)
		{
			if (tl->leafs_count == 1)
				trace->startsolid = true;
			return TR_SOLID;
		}
		else
		{
			if (num == CONTENTS_EMPTY)
				trace->inopen = true;
			else
				trace->inwater = true;
			return TR_EMPTY;
		}
	}

	node = hull->clipnodes + num;

	plane = hull->planes + node->planenum;

	//printf("t_rht: %p %p %i %i %f ", node, plane, node->planenum, plane->type, plane->dist);
	//PRINT_VEC(plane->normal);

	if (plane->type < 3)
	{
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		//printf("t_rht 1: t1, t2, plane->dist %f %f %f\n", t1, t2, plane->dist);
	}
	else
	{
		t1 = Vector_DotProduct(plane->normal, p1) - plane->dist;
		t2 = Vector_DotProduct(plane->normal, p2) - plane->dist;
		//printf("t_rht 2: t1, t2, plane->dist %f %f %f\n", t1, t2, plane->dist);
	}

	if (t1 >= 0 && t2 >= 0)
	{
		num = node->children[0];
		//printf("going to start 1\n");
		goto start;
	}

	if (t1 < 0 && t2 < 0)
	{
		num = node->children[1];
		//printf("going to start 2\n");
		goto start;
	}

	frac = t1 / (t1 - t2);
	frac = bound(0, frac, 1);
	midf = p1f + (p2f - p1f) * frac;

	for (i=0; i<3; i++)
		mid[i] = p1[i] + frac * (p2[i] - p1[i]);

	nearside = (t1 < t2) ? 1 : 0;
	//printf("doing trace 1 %f %f\n", frac, midf);
	check = Trace_RecursiveHullTrace(tl, node->children[nearside], p1f, midf, p1, mid);
	if (check == TR_BLOCKED)
	{
		return check;
	}

	if (check == TR_SOLID && (trace->inopen  || trace->inwater))
	{
		return check;
	}
	oldcheck = check;

	//printf("doing trace 2\n");
	check = Trace_RecursiveHullTrace(tl, node->children[1 - nearside], midf, p2f, mid, p2);
	if (check == TR_EMPTY || check == TR_BLOCKED)
	{
		return check;
	}

	if (oldcheck != TR_EMPTY)
	{
		return check;
	}

	if (!nearside)
	{
		Vector_Copy(trace->plane.normal, plane->normal);
		trace->plane.dist = plane->dist;
	}
	else
	{
		Vector_Negate(trace->plane.normal, plane->normal);
		trace->plane.dist = -plane->dist;
	}

	if (t1 < t2)
		frac = (t1 + DIST_EPSILON) / (t1 - t2);
	else
		frac = (t1 - DIST_EPSILON) / (t1 - t2);

	frac = bound(0, frac, 1);

	midf = p1f + (p2f - p1f) * frac;

	for (i=0; i<3; i++)
		mid[i] = p1[i] + frac * (p2[i] - p1[i]);

	trace->fraction = midf;

	//printf("fraction: %f\n", midf);

	Vector_Copy(trace->endpos, mid);

	return TR_BLOCKED;
}

struct trace *Trace_HullTrace(struct trace *trace, struct hull *hull, vec3_t start, vec3_t end)
{
	int check;
	struct trace_local tl;
	struct trace *lt;

	if (hull == NULL)
		return NULL;

	if (trace == NULL)
		lt = calloc(1, sizeof(*lt));
	else
	{
		lt = trace;
		memset(trace, 0, sizeof(*trace));
	}

	if (lt == NULL)
		return NULL;

	memset(&tl, 0, sizeof(*&tl));

	tl.trace = lt;
	lt->fraction = 1;
	lt->startsolid = false;
	Vector_Copy(lt->endpos, end);
	tl.hull = hull;

	check = Trace_RecursiveHullTrace(&tl, hull->firstclipnode, 0, 1, start, end);

	if (check == TR_SOLID)
	{
		lt->startsolid = lt->allsolid = true;
		Vector_Copy(lt->endpos, start);
	}

	return lt;
}

struct bounding_box
{
	vec3_t mins, maxs;
};

static struct bounding_box *create_bounding_box(struct bounding_box *bbox, vec3_t mins, vec3_t maxs, vec3_t start, vec3_t end)
{
	struct bounding_box *box;
	int i;

	if (bbox)
		box = bbox;
	else
	{
		box = calloc(1, sizeof(*box));
		if (box == NULL)
			return NULL;
	}

	for (i=0; i<3; i++)
	{
		if (end[i] > start[i])
		{
			box->mins[i] = start[i] + mins[i] - 1;
			box->maxs[i] = end[i] + maxs[i] + 1;
		}
		else
		{
			box->mins[i] = end[i] + mins[i] - 1;
			box->maxs[i] = start[i] + maxs[i] + 1;
		}
	}

	return box;
}

struct trace *Trace_ClipMoveToEdict(struct server *server, struct edict *edict, vec3_t mins, vec3_t maxs, vec3_t start, vec3_t stop)
{
	struct trace *trace;
	struct hull *hull;
	vec3_t offset, start_l, stop_l;

	if (!server || !edict)
		return NULL;

	hull = Server_HullForEdict(server, edict, mins, maxs, offset);
	/*
	printf("hull ptr1: %p\n", &server->map->submodels[0].hulls[1]);
	printf("hull ptr2: %p\n", hull);
	PRINT_VEC(hull->clip_mins);
	PRINT_VEC(hull->clip_maxs);
	printf("offset: ");
	PRINT_VEC(offset);
	*/

	Vector_Subtract(start_l, start, offset);
	Vector_Subtract(stop_l, stop, offset);
	/*
	printf("start_l: ");
	PRINT_VEC(start_l);

	printf("stop_l: ");
	PRINT_VEC(stop_l);
	*/

	trace = Trace_HullTrace(NULL, hull, start_l, stop_l);

	Vector_Add(trace->endpos, trace->endpos, offset);

	if (trace->fraction < 1 || trace->allsolid)
		trace->e.ent = edict;

	/*
	printf("endpos: ");
	PRINT_VEC(trace->endpos);
	*/

	return trace;
}

#warning fix
#warning really fix!
struct trace *Trace_Trace(struct server *server, struct trace *trace_in, vec3_t mins, vec3_t maxs, vec3_t start, vec3_t stop, int type, struct edict *pass_edict)
{
	struct trace *trace;

	//trace through the level
	//printf("trace starting ---------------------------\n");
	//printf("ptr: %p %p\n", server->map->planes, server->map->planes + server->map->planes_count);
	trace = Trace_ClipMoveToEdict(server, server->edicts, mins, maxs, start, stop);
	//printf("%i\n", server->map->planes_count);
	//printf("trace endet ------------------------------\n");

	return trace;
}

/*
struct trace *Trace_Trace(struct server *server, struct trace *trace_in, vec3_t mins, vec3_t maxs, vec3_t start, vec3_t end, int type, struct edict *pass_edict)
{
	struct bounding_box box;
	struct edict *edicts[MAX_EDICTS];
	struct edict *touch;
	struct trace *trace, *trace1;
	int touches, i;

	// create bounding box
	if (create_bounding_box(&box, mins, maxs, start, end) == NULL)
		return NULL;

	if (trace_in == NULL)
		trace = calloc(1, sizeof(*trace));
	else
		trace = trace_in;

	if (trace == NULL)
		return NULL;

	touches = World_AreaEdicts(server, box.mins, box.maxs, edicts, MAX_EDICTS, AREA_SOLID);

	for (i=0; i<touches; i++)
	{
		if (trace->allsolid)
			break;

		touch = edicts[i];

		if (touch == pass_edict)
			continue;

#warning do a trigger check here

		if ((type & MOVE_NOMONSTERS) && touch->solid != SOLID_BSP)
			continue;

#warning do point check here
//		if (clip->pass_edict && clip->pass_edict->

#warning do missile check
		if (pass_edict)
		{
		}

		trace1 = Trace_ClipMoveToEdict(server, NULL, touch, mins, maxs, start, end);
	}
}
*/
