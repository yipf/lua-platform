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
	stream2scene(stdin,element,light);
	scene2stream(element,light,stdout);
	
	vec4 org,dir,cp,color,temp,ref,front_color,back_color,cur_color;
	scalar dx,dy,xx,hw,hh;
	CREATE_VEC4(org)
	CREATE_VEC4(dir)
	CREATE_VEC4(cp)
	CREATE_VEC4(color)
	CREATE_VEC4(temp)
	CREATE_VEC4(ref)
	CREATE_VEC4(front_color)
	CREATE_VEC4(back_color)
	
	front_color[0]=1.0; 	front_color[1]=1.0; 	front_color[2]=1.0; 	
	back_color[0]=0.1; 	back_color[1]=0.0; 	back_color[2]=0.1; 	
	
	int width,height,x,y;
	int comp;
	char* data;
	width=2<<8;
	height=2<<8;
	comp=3;
	data=create_img_data(width,height,comp);
	dx=1.0/width; dy=1.0/height;
	//~ org[0]=0.0; 	org[1]=0.0; 	org[2]=1.0; 	
	//~ dir[0]=0.0; 	dir[1]=0.0; 	dir[2]=-1.0; 	
	//~ color=ray2color(org,dir, element,  light,  cp, color,  temp,  front_color,  back_color);
	//~ SHOW_VEC4(color)
	for(x=0;x<width;x++){
		xx=dx*x;
		for(y=0;y<height;y++){
			/* init ray */
			org[0]=xx; 	org[1]=(height-y)*dy; 	org[2]=1.0; 	
			dir[0]=0.0; 	dir[1]=0.0; 	dir[2]=-1.0; 	
			CLONE(front_color,color,comp); /* init color with front color */
			cur_color=ray2color(org,dir, element,  light,  cp, color,  ref, temp);
			fill_color(cur_color?cur_color:back_color,data+(y*width+x)*comp,comp);
		}
	}
	stbi_write_png ("scenes/scene.png", width, height,comp, data, width*comp);
	printf("\n");
	return 0;
}