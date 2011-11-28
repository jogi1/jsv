#ifndef VECTOR_H
#define VECTOR_H

#define Vector_DotProduct(x, y)		(x[0]*y[0] + x[1]*y[1] + x[2]*y[2])
#define Vector_Subtract(c,a,b)	(c[0]=a[0]-b[0],c[1]=a[1]-b[1],c[2]=a[2]-b[2])
#define VectorCopy(b,a)(b[0]=a[0],b[1]=a[1],b[2]=a[2])


void Vector_Scale (vec3_t in, vec_t scale, vec3_t out);
float Vector_Normalize (vec3_t v);
void Vector_Angles(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void Vector_Add(vec3_t out, vec3_t a, vec3_t b);
void Vector_Clear(vec3_t out);
void Vector_Set(vec3_t in, float x, float y, float z);
#endif
