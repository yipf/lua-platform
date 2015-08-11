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
	SCALE(s,a,n)
	return n;
}

vec4 reflect(vec4 dir,vec4 normal,vec4 ref){
	scalar s;
	s=2*DOT(dir,normal);
	EXP3(dir,-,s*normal,ref)
	return ref;
}

element_t create_element(void ){
	return (element_t)malloc(sizeof(element_));
}

/* compute the nearest point pair <p1,p2> of  line1 <p1,d1> and line2 <p2,d2>, return a vec4 <x1,x2,other1,other2> where cp1=p1+x1*d1, cp2=p2+x2*d2, */
vec4 line_hit_line(vec4 p1,vec4 d1,vec4 p2,vec4 d2,vec4 pair,vec4 failed){
	scalar a1,b1,a2,b2,c1,c2,n;
	/* compute middle variables to solve the problem: (cp1-cp2)*d1=0 &&	(cp1-cp2)*d2=0	*/
	EXP3(p2,-,p1,pair)
	a1=DOT(d1,d1);	b1=DOT(d2,d1); 	c1=DOT(pair,d1);  /* a1*x1-b1*x2=c1 */
	a2=b1;	b2=DOT(d2,d2);	c2=DOT(pair,d2);/* a2*x1-b2*x2=c2, where a2=dot(d1,d2)=dot(d2,d1)=b1; */
	n=(a1*b2-a2*b1);
	if(n==0.0){return failed;}
	pair[0]=(c1*b2-c2*b1)/n;
	pair[1]=(c1*a2-c2*a1)/n;
	return pair;
}

/* compute the corss point cp of a ray <org,dir> to triangle ABC with normal N, if no cross points, return failed */
vec4 ray_hit_triangle(vec4 org, vec4 dir, vec4 A, vec4 BA, vec4 CA, vec4 N, vec4 cp, vec4 failed){
	scalar t1,t2,a1,a2,b1,b2,c1,c2;
	t1=DOT(dir,N);
	EXP3(A,-,org,cp)
	t2=DOT(cp,N);
	if(t2<0){/* if (A-ray.from)*tri.normal<0 then it is possible to cross  */
		t1=t1/t2;
		EXP3(org,+,t1*dir,cp) 	/* compute the cross point of ray and the plane defined by the triangle ABC */
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
	EXP3(org,-,sph,cp)
	t=DOT(cp,cp);
	if(t<=rr){ 	/* printf("\nOrg inside the sphere!"); */		return failed;	}
	t=DOT(cp,dir);
	if(t>0){ 	/* printf("\nSphere is behind the ray!"); */	return failed;	}
	EXP3(cp,-,t*dir,cp)
	cp[3]=t;
	t=DOT(cp,cp);
	if(t>rr){ /* printf("\nDistance between center and the ray is larger then radius!"); */	return failed;	}
	t=-cp[3]-sqrt(rr-t);
	EXP3(org,+,t*dir,cp)
	cp[3]=t;
	return cp;
}

vec4 ray_hit_and_reflect_sphere(vec4 org, vec4 dir,vec4 sph,vec4 cp, vec4 ref, vec4 failed){
	cp=ray_hit_sphere(org, dir,sph,cp, failed);
	if(cp!=failed){
			/* compute the reflect ray direction and store it in ref */
		EXP3(cp,-,sph,ref)
		normalize(ref,ref);
		reflect(dir,ref,ref);
	}
	return cp;
}

vec4 ray_hit_and_reflect_triangle(vec4 org, vec4 dir, vec4 A, vec4 BA, vec4 CA, vec4 N, vec4 cp, vec4 ref, vec4 failed){
	cp=ray_hit_triangle(org, dir, A, BA, CA, N, cp, failed);
	if(cp!=failed){
			/* compute the reflect ray direction and store it in ref */
		reflect(dir,N,ref);
	}
	return cp;
}

vec4 ray_hit_elements(vec4 org,vec4 dir,element_t element, vec4 cp, vec4 ref, vec4 temp){
	scalar* data;
	vec4 cur;
	cp[3]=HUGE_VAL;
	while((element=element->next)&&(data=element->data)){ /* For each element in elements*/
		switch(element->type){
			case SPHERE:
				cur=ray_hit_and_reflect_sphere(org, dir, data, temp, ref, 0);
				break;
			case TRIANGLE:
				cur=ray_hit_and_reflect_triangle(org, dir, data, data+3, data+6, data+9, temp, ref, 0);
				break;
			default: 
				cur=0;
				break;
		}
		if(cur==temp && cur[3]<cp[3]){ /* if find a nearer cross point cur, update cp with cur */
			CLONE(cur,cp,4); 
		}
	}
	return cp[3]==HUGE_VAL?0:cp;
}

vec4 ray_hit_any_element(vec4 org,vec4 dir,element_t element, vec4 temp){
	scalar* data;
	vec4 cur;
	while((element=element->next)&&(data=element->data)){ /* For each element in elements*/
		switch(element->type){
			case SPHERE:
				cur=ray_hit_sphere(org, dir, data, temp, 0);
				break;
			case TRIANGLE:
				cur=ray_hit_triangle(org, dir, data, data+3, data+6, data+9, temp, 0);
				break;
			default: break;
		}
		if(cur==temp) return cur;
	}
	return 0;
}

vec4 ray2color(vec4 org,vec4 dir, element_t element, element_t light, vec4 cp,vec4 color, vec4 ref, vec4 temp){
	scalar rr,power;
	vec4 data;
	int changed;
	changed=0;
	/* STEP1: compute the nearest cross point, and store it to cp */
	org=ray_hit_elements(org,dir,element,cp,ref, temp);
	if(!org){ return 0;} /* if no cross point then return back_color */
	/* STEP2: compute color according to cross point */
	while((light=light->next)&&(data=light->data)){ /* For each element in lights */
		rr=data[3];
		if(rr==0.0){ /* if the light is parallel light source */
			power=DOT(data,ref);
			if(power>0 && !ray_hit_any_element(org,data,element,temp)){ /* if the ray from cross point can reach the light source straightforward */
				data=light->color;
				EXP3(color,*,power*data,color)
				changed+=1;
			}
		}else{ /* if the light is point light source */
			EXP3(data,-,org,dir)
			normalize(dir,dir);
			if(!ray_hit_any_element(org,dir,element,temp)) {  /* if the ray from cross point can reach the light source straightforward */
				data=ray_hit_sphere(org, dir, data, cp, 0);
				if(data==cp){ /* if the ray is from light */
					power=rr/data[3]/data[3];
					data=light->color;
					EXP3(color,*,power*data,color)
					changed+=1;
				}
			}
		}
	}
	return changed?color:0;
}