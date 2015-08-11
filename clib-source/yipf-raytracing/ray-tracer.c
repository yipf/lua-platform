#include "ray-tracer.h"
#include <math.h>
#include <string.h>

int show_vec4(vec4 vec,int n){
	int i;
	printf("[\t");
	for(i=0;i<n;++i){ printf("%f\t",vec[i]);}
	printf("]");
	return 0;
}

vec4 cross(vec4 a,vec4 b,vec4 c){
	scalar x1,y1,z1,x2,y2,z2;
	x1=a[0];	y1=a[1];	z1=a[2];	
	x2=b[0];	y2=b[1];	z2=b[2];
	c[0]=y1*z2-y2*z1;
	c[1]=z1*x2-z2*x1;
	c[2]=x1*y2-x2*y1;
	return c;
}

scalar norm(vec4 v){
	return sqrt(DOT(v,v));
}

vec4 normalize(vec4 a,vec4 n){
	scalar s;
	s=1.0/norm(a);
	EXP2(n,=,s*a)
	return n;
}

vec4 regularize(vec4 a,vec4 r){
	scalar s;
	s=a[0];
	s=MAX(s,a[1]);
	s=MAX(s,a[2]);
	if(s>1.0){
		s=1.0/s; 
		EXP2(r,=,s*a)
	}
	return r;
}

vec4 reflect(vec4 dir,vec4 normal,vec4 ref){
	scalar s;
	s=2*DOT(dir,normal);
	EXP3(ref,=,dir,-,s*normal)
	return ref;
}

element_t create_element(void ){
	return (element_t)malloc(sizeof(element_));
}

/* compute the nearest point pair <p1,p2> of  line1 <p1,d1> and line2 <p2,d2>, return a vec4 <x1,x2,other1,other2> where cp1=p1+x1*d1, cp2=p2+x2*d2, */
vec4 line_hit_line(vec4 p1,vec4 d1,vec4 p2,vec4 d2,vec4 pair,vec4 failed){
	scalar a1,b1,a2,b2,c1,c2,n;
	/* compute middle variables to solve the problem: (cp1-cp2)*d1=0 &&	(cp1-cp2)*d2=0	*/
	EXP3(pair,=,p2,-,p1)
	a1=DOT(d1,d1);	b1=DOT(d2,d1); 	c1=DOT(pair,d1);  /* a1*x1-b1*x2=c1 */
	a2=b1;	b2=DOT(d2,d2);	c2=DOT(pair,d2);/* a2*x1-b2*x2=c2, where a2=dot(d1,d2)=dot(d2,d1)=b1; */
	n=(a1*b2-a2*b1);
	if(n==0.0){return failed;}
	pair[0]=(c1*b2-c2*b1)/n;
	pair[1]=(c1*a2-c2*a1)/n;
	return pair;
}


vec4 ray_hit_plane(vec4 org, vec4 dir,vec4 point,vec4 N,vec4 cp,vec4 failed){
	scalar t1,t2;
	t1=DOT(dir,N);
	t2=DOT(point,N)-DOT(org,N);
	if(t1<0.0 && t2<0.0){ /* if (point-ray.org)*plane.normal<0 then it is possible to cross  */
		t1=t2/t1;
		EXP3(cp,=,org,+,t1*dir) 
		cp[3]=t1;
		return cp;
	}
	return 0;
}

/* compute the corss point cp of a ray <org,dir> to triangle ABC with normal N, if no cross points, return failed */
vec4 ray_hit_triangle(vec4 org, vec4 dir, vec4 A, vec4 BA, vec4 CA, vec4 N, vec4 cp, vec4 failed){
	scalar t1,t2,a1,a2,b1,b2,c1,c2;
	t1=DOT(dir,N);
	EXP3(cp,=,A,-,org)
	t2=DOT(cp,N);
	if( t1<0.0 && (t1=t2/t1)>0.0){/* if cross is valid  */
		EXP3(cp,=,org,+,t1*dir) 	/* compute the cross point of ray and the plane defined by the triangle ABC */
		cp[3]=t1;
		/*test if the cross point is inside the tri, where cp=A+t1(C-A)+t2(B-A) and if t1>=0, t2>=0, t1+t2<=1 then P is inside ABC*/
		a1=CA[0]; 	b1=BA[0];		c1=cp[0]-A[0]; 	/*	a1*x+b1*y=c1 	*/
		a2=CA[1]; 	b2=BA[1];		c2=cp[1]-A[1]; 	/* a2*x+b2*y=c2  */
		t2=a1*b2-a2*b1;
		if(t2==0.0){return failed;}
		t1=(c1*b2-c2*b1)/t2; 	t2=(c2*a1-c1*a2)/t2;
		return (t1>=0.0 && t2>=0.0 && t1+t2<=1.0)?cp:failed;
	}
	return failed;
}

/* compute the corss point cp of a ray <org,dir> to a sphere <x,y,z,r^2> cectered at (x,y,z) with radius r, if no cross points, return failed */
vec4 ray_hit_sphere(vec4 org, vec4 dir,vec4 sph,vec4 cp, vec4 failed){
	scalar t,rr;
	rr=sph[3];
	EXP3(cp,=,org,-,sph)
	t=DOT(cp,cp);
	if(t<=rr){ 	/* printf("\nOrg inside the sphere!"); */		return failed;	}
	t=DOT(cp,dir);
	if(t>0){ 	/* printf("\nSphere is behind the ray!"); */	return failed;	}
	EXP2(cp,-=,t*dir)
	cp[3]=t;
	t=DOT(cp,cp);
	if(t>rr){ /* printf("\nDistance between center and the ray is larger then radius!"); */	return failed;	}
	t=-cp[3]-sqrt(rr-t);
	EXP3(cp,=,org,+,t*dir)
	cp[3]=t;
	return cp;
}

vec4 get_reflect(vec4 cp,vec4 dir, element_t element, vec4 ref){
	scalar* data;
	data=element->data;
	switch(element->type){
		case SPHERE:
			EXP3(ref,=,cp,-,data)
			normalize(ref,ref);
			reflect(dir,ref,ref);
			break;
		case TRIANGLE:
			reflect(dir,data+9,ref);
			break;
		case PLANE:
			reflect(dir,data+3,ref);
			break;
		default: break;
	}
	return ref;
}


element_t ray_hit_elements(vec4 org,vec4 dir,element_t element, vec4 cp, vec4 temp){
	scalar* data;
	vec4 cur;
	cp[3]=HUGE_VAL;
	element_t cur_element;
	cur_element=0;
	while((element=element->next)&&(data=element->data)){ /* For each element in elements*/
		switch(element->type){
			case SPHERE:
				cur=ray_hit_sphere(org, dir, data, temp, 0);
				break;
			case TRIANGLE:
				cur=ray_hit_triangle(org, dir, data, data+3, data+6, data+9, temp, 0);
				break;
			case PLANE:
				cur=ray_hit_plane(org, dir, data, data+3, temp, 0);
				break;
			default: 
				cur=0;
				break;
		}
		if(cur==temp && cur[3]<cp[3]){ /* if find a nearer cross point cur, update cp with cur */
			cur_element=element;
			CLONE(cur,cp,4); 
		}
	}
	return cur_element;
}

element_t ray_hit_any_element(vec4 org,vec4 dir,element_t element, vec4 cp){
	scalar* data;
	vec4 cur;
	while((element=element->next)&&(data=element->data)){ /* For each element in elements*/
		switch(element->type){
			case SPHERE:
				cur=ray_hit_sphere(org, dir, data, cp, 0);
				break;
			case TRIANGLE:
				cur=ray_hit_triangle(org, dir, data, data+3, data+6, data+9, cp, 0);
				break;
			case PLANE:
				cur=ray_hit_plane(org, dir, data, data+3, cp, 0);
				break;
			default: 
				break;
		}
		if(cur==cp) return element;
	}
	return 0;
}

vec4 ray2color(vec4 org,vec4 dir, element_t element, element_t light, vec4 cp,vec4 color, vec4 ref, vec4 temp,vec4 back_color){
	scalar rr,power;
	vec4 data,e_color;
	element_t cur_element;
	/* STEP1: compute the nearest cross point, and store it to cp */
	cur_element=ray_hit_elements(org,dir,element,cp,temp);
	if(!cur_element){ return 0;} /* if no cross point then return back_color */
	/* STEP2: compute color according to cross point */
	get_reflect(cp,dir, cur_element, ref); /* compute the reflect direction according to element types */
	e_color=cur_element->color;
	while((light=light->next)&&(data=light->data)){ /* For each element in lights,  */
		rr=data[3];
		power=0.0;
		if(rr==0.0){ /* if the light is parallel light source, */
			if(ray_hit_any_element(cp,data,element,temp)){ /* if there are something lay on the path from ray to the light source */
				continue;
			}
			power=DOT(data,ref);
		}else{ /* if the light is point light source, added later ... */
			if(ray_hit_any_element(cp,data,element,temp)){ /* if there are something lay on the path from ray to the light source */
				continue;
			}
			EXP3(temp,=,data,-,org)
			normalize(temp,temp);
			data=ray_hit_sphere(org, temp, data, cp, 0);
			if(data==cp){ /* if the ray is from light */
				power=rr/data[3]/data[3];
			}
		}
		if(power<=0.0) continue; 
		/* when the light is valid, update color, where color=color +  power * e_color * light_color  */
		data=light->color;
		EXP3(color,+=,e_color,*,power*data) 
	}
	regularize(color,color);
	return color;
}