#include "io.h"
#include <ctype.h>
#include <string.h>

scalar* fill_data(FILE* stream,scalar* dst,unsigned int length){
	unsigned int i;
	int ret;
	for(i=0;i<length;i++){
		ret=fscanf(stream,"%lf",dst+i);
	}
	return dst;
}

element_t append_element(element_t current,int type, scalar* data, scalar* color){
	current->next=create_element();
	current=current->next;
	current->type=type;
	current->data=data;
	current->color=color;
	return current;
}

int stream2scene(FILE* stream,element_t element,element_t light,scalar* output_config, scalar* view){
	char ch;
	scalar *data,*color,*ptr,*v1,*v2,*v3;
	scalar temp;
	color=ALLOC_SCALAR(4);
	color[0]=0.0; color[1]=0.0; color[2]=0.0; color[3]=0.0;
	ch=fgetc(stream);
	while(ch!=EOF){
		switch(ch){
			/* global set up */
			case 'O': /* output setting, reading width+height+depth+dx+dy=5 floats. */
				fill_data(stream,output_config,5);
				break;
			case 'V': /* view setting, reading From+To+UP+1=10 floats. */
				fill_data(stream,view,9);
				ptr=view; 		/* from */
				v1=view+3; 	/* to */
				v2=view+6; 	/* UP */
				v3=view+9; 	/* -Z */
				EXP3(v3,=,v1,-,ptr) /* -Z=to-from */
				cross(v3,v2,v1); /* X=cross(-Z,UP) */
				cross(v1,v3,v2); /* Y=cross(X,-Z)*/
				normalize(v1,v1); 	normalize(v2,v2); 	normalize(v3,v3);
				fill_data(stream,view+12,1); /* fact */
				break;
			case 'L': /* light, reading Pos+model=4 floats. */
				data=ALLOC_SCALAR(4);
				data=fill_data(stream,data,4);
				if(data[3]==0.0) { normalize(data,data);} /* if a parallel light source*/
				/* append to current light list */
				light=append_element(light,LIGHT,data,color);
				break;
			/* elements */
			case 'P': /* plane, reading Pos+Dir=6 floats. */
				data=ALLOC_SCALAR(6);
				data=fill_data(stream,data,6);
				ptr=data+3;	normalize(ptr,ptr);
				/* append to current element list */
				element=append_element(element,PLANE,data,color);
				break;
			case 'T': /* triangle, reading A+B+C=9 floats. */
				data=ALLOC_SCALAR(12);
				data=fill_data(stream,data,9);
				ptr=data+3; EXP3(ptr,=,ptr,-,data) /* B-A */
				ptr=data+6; EXP3(ptr,=,ptr, -,data) /* C-A */
				cross(data+3,data+6,data+9); /* compute normal */
				ptr=data+9;	normalize(ptr,ptr);
				/* append to current element list */
				element=append_element(element,TRIANGLE,data,color);
				fill_data(stream,&temp,1);
				SHOW(temp,"%f")
				if(temp!=0.0){ /* if the thick of Triangle is not 0.0, append a new triangle */
					v1=data;
					v2=data+3;
					v3=data+6;
					ptr=data+9;
					data=ALLOC_SCALAR(12);
					EXP3(data,=,v1,-,temp*ptr)
					v1=data+3; 	CLONE(v3,v1,3)
					v1=data+6; 	CLONE(v2,v1,3)
					v1=data+9; 	EXP2(v1,=,-ptr)
					element=append_element(element,TRIANGLE,data,color);
				}
				break;
			case 'S': /* sphere, reading Center+r=4 floats. */
				data=ALLOC_SCALAR(4);
				data=fill_data(stream,data,4);
				/* append to current element list */
				element=append_element(element,SPHERE,data,color);
				break;
			case 'C': /* if color, then update the current color, reading r+g+b=3 floats. */
				color=ALLOC_SCALAR(4);
				color=fill_data(stream,color,3);
				break;
			case '#': /* if input a '#', goto next line */
				while(((ch=fgetc(stream))!=EOF)&&ch!='\n');
				break;
			default:
				break;
		}
		while(((ch=fgetc(stream))!=EOF)&&!isprint(ch)); /* goto nextvalid character */
	}
	return 0;
}

int data2stream(scalar* data,FILE* stream,unsigned int length){
	unsigned int i;
	fprintf(stream,"\t[");
	for(i=0;i<length;i++){		fprintf(stream,"\t%f",data[i]);	}
	fprintf(stream,"\t]");
	return 0;
}

int scene2stream(element_t element,element_t light,FILE* stream){
	unsigned int count;
	scalar* data;
	/* print lights */
	fprintf(stream,"\nLIGHTS:");
	count=0;
	while((light=light->next) && (data=light->data)){
		fprintf(stream,"\n%d",++count);
		fprintf(stream,"\t%s",data[3]==0.0?"Parallel":"Point");
		fprintf(stream,"\n\tPos:");  data2stream(data,stream,3);
		fprintf(stream,"\n\tColor:");  data2stream(light->color,stream,3);
	}
	/* print elements */
	fprintf(stream,"\nELEMENTS:");
	count=0;
	while((element=element->next)&&(data=element->data) ){
		fprintf(stream,"\n%d",++count);
		switch(element->type){
			case TRIANGLE:
				fprintf(stream,"\tTriangle");
				fprintf(stream,"\n\tA:");  data2stream(data,stream,3);
				fprintf(stream,"\n\tBA:");  data2stream(data+3,stream,3);
				fprintf(stream,"\n\tCA:");  data2stream(data+6,stream,3);
				fprintf(stream,"\n\tNormal:");  data2stream(data+9,stream,3);
				fprintf(stream,"\n\tColor:");  data2stream(element->color,stream,3);
				break;
			case SPHERE:
				fprintf(stream,"\tSphere");
				fprintf(stream,"\n\tCenter:");  data2stream(data,stream,3);
				fprintf(stream,"\n\tR^2:\t%f",data[3]); 
				fprintf(stream,"\n\tColor:");  data2stream(element->color,stream,3);
				break;
			case PLANE:
				fprintf(stream,"\tPlane");
				fprintf(stream,"\n\tPoint:");  data2stream(data,stream,3);
				fprintf(stream,"\n\tNormal:"); data2stream(data+3,stream,3);
				fprintf(stream,"\n\tColor:");  data2stream(element->color,stream,3);
				break;
			default: 
				break;
		}
	}
	return 0;
}

char* create_image_data(element_t element, element_t light, scalar* image,scalar* view){
	
}


char* create_img_data(unsigned int width,unsigned int height,int comp){
	return (char*)malloc(width*height*comp);
}

//~ int save_img(char* data, const char* filepath,unsigned int width, unsigned int height, int comp){
	//~ return stbi_write_png (filepath, width, height,comp, data, width*comp);
//~ }