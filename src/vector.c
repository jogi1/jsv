#include "server.h"
#define	PITCH	0 // up / down
#define	YAW	1 // left / right
#define	ROLL	2 // fall over

#ifndef M_PI
#define M_PI	3.14159265358979323846	// matches value in gcc v2 math.h
#endif

void Vector_Angles(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float angle;
	float sr, sp, sy, cr, cp, cy;
	
	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}

	if (right || up)
	{
		angle = angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		if (right)
		{
			right[0] = (-1*sr*sp*cy+-1*cr*-sy);
			right[1] = (-1*sr*sp*sy+-1*cr*cy);
			right[2] = -1*sr*cp;
		}

		if (up)
		{
			up[0] = (cr*sp*cy+-sr*-sy);
			up[1] = (cr*sp*sy+-sr*cy);
			up[2] = cr*cp;
		}
	}
}

float Vector_Normalize (vec3_t v)
{
	float length, ilength;

	length = sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (length)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}

void Vector_Scale (vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

void Vector_Add(vec3_t out, vec3_t a, vec3_t b)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}
