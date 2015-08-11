#include "ray-tracer.h"
#include "io.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <math.h>

int fill_color(scalar f[],char data[],unsigned int n){
 	unsigned int i;
	for(i=0;i<n;i++){
		data[i]=floor(f[i]*255.0);
	}
	return 0;
}

int main(){
	element_t element=create_element();
	element_t light=create_element();
	scalar image[]={256,256,3,1.0,1.0};
	scalar view[]={0.0,0.0,3.0,	1.0, 0.0,0.0,	0.0,1.0,0.0,		0.0,0.0,-1.0, 	1.0 };
	//~ stream2scene(stdin,element,light,view,window);
	stream2scene(stdin,element,light,image,view);
	scene2stream(element,light,stdout);
	/* define temperal variables */
	vec4 org,dir,cp,color,temp,ref,front_color,back_color,cur_color,X,Y,Z;
	CREATE_VEC4(org)
	CREATE_VEC4(dir)
	CREATE_VEC4(cp)
	CREATE_VEC4(color)
	CREATE_VEC4(temp)
	CREATE_VEC4(ref)
	CREATE_VEC4(front_color)
	CREATE_VEC4(back_color)
	
	front_color[0]=1.0; 	front_color[1]=1.0; 	front_color[2]=1.0; 	
	back_color[0]=0.1; 	back_color[1]=0.1; 	back_color[2]=0.1; 	
	
	int width,height,x,y,comp;
	char* data;
	scalar dx,dy,xx,yy,zz,hh,hw,s;
	width=floor(image[0]);
	height=floor(image[1]);
	comp=floor(image[2]);
	data=create_img_data(width,height,comp);
	dx=image[3]/image[0]; dy=image[4]/image[1];
	hw=image[3]*0.5; 	hh=image[4]*0.5;
	X=view+3; 	
	Y=view+6; Z=view+9; 
	s=view[12];
	for(x=0;x<width;x++){
		xx=dx*x-hw;
		for(y=0;y<height;y++){
			/* init ray */
			yy=(height-y)*dy-hh;
			/* set org to view*/
			CLONE(view,org,3)
			/* dir=xx*X+yy*Y+(-1.0)*Z*/
			EXP2(dir,=,xx*X)
			EXP2(dir,+=,yy*Y)
			EXP2(dir,+=,s*Z)
			normalize(dir,dir);
			//~ SHOW_VEC4(org)
			//~ SHOW_VEC4(dir)
			//~ org[0]=xx; 	org[1]=yy; 	org[2]=1.0; 	
			//~ dir[0]=0.0; 	dir[1]=0.0; 	dir[2]=-1.0; 	
			color[0]=0.0; 	color[1]=0.0; 	color[2]=0.0; 	
			
			cur_color=ray2color(org,dir, element,  light,  cp, color,  ref, temp,back_color);
			fill_color(cur_color?cur_color:back_color,data+(y*width+x)*comp,comp);
		}
	}
	stbi_write_png ("scenes/scene.png", width, height,comp, data, width*comp);
	printf("\n");
	return 0;
}