#ifndef TRACE_S_H
#define TRACE_S_H

struct trace
{
	qboolean allsolid, startsolid, inopen, inwater;
	float fraction;
	vec3_t endpos;
	struct plane plane;
	union {
		int entnum;
		struct edict *ent;
	}e;
};

#endif
