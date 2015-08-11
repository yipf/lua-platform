#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include <stdio.h>
#include <stdlib.h>

typedef double scalar;
typedef scalar* vec4;

#define DEBUG

#ifdef DEBUG
	#define SHOW(var,format) printf("\n%s:%d:\t%s\t:: ",__FILE__,__LINE__,#var);printf(format,var);
	#define SHOW_VEC4(vec) printf("\n%s:%d:\t%s\t:: ",__FILE__,__LINE__,#vec);show_vec4(vec,4);
#else
	#define SHOW(var,format) 
	#define SHOW_VEC4(vec)
#endif

#define EXP3(C, eq, A, op, B ) C[0] eq A[0] op B[0];	C[1] eq A[1] op B[1];	C[2] eq A[2] op B[2];

#define EXP2(B, eq, A) B[0] eq A[0];	B[1] eq A[1]; 	B[2] eq A[2];

#define CLONE(src,dst,n) memcpy(dst,src,sizeof(scalar)*n);

#define ALLOC_SCALAR(length) 	(scalar*)malloc(sizeof(scalar)*length)
#define CREATE_VEC4(A) 	A=(scalar*)malloc(sizeof(scalar)<<2);

#define DOT(A,B) 	(A[0]*B[0]+A[1]*B[1]+A[2]*B[2])

#define MAX(a,b) ((a)>(b))?(a):(b)

vec4 cross(vec4 a,vec4 b,vec4 c);
scalar norm(vec4 v);
vec4 normalize(vec4 a,vec4 n);
vec4 regularize(vec4 a,vec4 r);
vec4 reflect(vec4 dir,vec4 normal,vec4 ref);



int show_vec4(vec4 vec,int n);

enum ELEMENT_TYPES{TRIANGLE,SPHERE,PLANE,LIGHT};

struct element_;

typedef struct element_{
	int type;
	scalar* data;
	vec4 color;
	struct element_ * next;
}element_,*element_t;

element_t create_element();

vec4 line_hit_line(vec4 p1,vec4 d1,vec4 p2,vec4 d2,vec4 pair,vec4 failed);

vec4 ray_hit_triangle(vec4 org, vec4 dir, vec4 A, vec4 BA, vec4 CA, vec4 N, vec4 cp, vec4 failed);
vec4 ray_hit_sphere(vec4 org, vec4 dir,vec4 sph,vec4 cp,vec4 failed);
vec4 ray_hit_plane(vec4 org, vec4 dir,vec4 point,vec4 N,vec4 cp,vec4 failed);


element_t ray_hit_elements(vec4 org,vec4 dir,element_t element, vec4 cp, vec4 temp);
element_t ray_hit_any_element(vec4 org,vec4 dir,element_t element, vec4 temp);

vec4 get_reflect(vec4 cp,vec4 dir, element_t element, vec4 ref);

vec4 ray2color(vec4 org,vec4 dir, element_t element, element_t light, vec4 cp,vec4 color, vec4 ref, vec4 temp,vec4 back_color);

#endif