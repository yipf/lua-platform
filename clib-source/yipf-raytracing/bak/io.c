#include "io.h"
#include <ctype.h>

scalar* fill_data(FILE* stream,scalar* dst,unsigned int length){
	unsigned int i;
	int ret;
	for(i=0;i<length;i++){
		ret=fscanf(stream,"%lf",dst+i);
		SHOW(dst[i],"%f")
	}
	return dst;
}

int stream2scene(FILE* stream,element_t element,element_t light){
	char ch;
	scalar *data,*color,*ptr;
	color=ALLOC_SCALAR(4);
	color[0]=0.0; color[1]=0.0; color[2]=0.0; color[3]=0.0;
	ch=fgetc(stream);
	while(ch!=EOF){
		switch(ch){
			case 'T': /* triangle */
				data=ALLOC_SCALAR(12);
				data=fill_data(stream,data,9);
				ptr=data+3; EXP3(ptr,-,data,ptr) /* B-A */
				ptr=data+6; EXP3(ptr,-,data,ptr) /* C-A */
				cross(data+3,data+6,data+9); /* compute normal */
				normalize(data+9,data+9);
				/* append to current element list */
				element->next=create_element();
				element=element->next;
				element->type=TRIANGLE;
				element->data=data;
				element->color=color;
				break;
			case 'S': /* sphere */
				data=ALLOC_SCALAR(4);
				data=fill_data(stream,data,4);
				/* append to current element list */
				element->next=create_element();
				element=element->next;
				element->type=SPHERE;
				element->data=data;
				element->color=color;
				break;
			case 'L': /* light */
				data=ALLOC_SCALAR(4);
				data=fill_data(stream,data,4);
				if(data[3]==0) { normalize(data,data);} /* if a parallel light source*/
				/* append to current light list */
				light->next=create_element();
				light=light->next;
				light->type=LIGHT;
				light->data=data;
				light->color=color;
				break;
			case 'C': /* if color, then update the current color */
				color=ALLOC_SCALAR(4);
				color=fill_data(stream,color,3);
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
				fprintf(stream,"\n\tNornal:");  data2stream(data+9,stream,3);
				fprintf(stream,"\n\tColor:");  data2stream(element->color,stream,3);
				break;
			case SPHERE:
				fprintf(stream,"\tSphere");
				fprintf(stream,"\n\tCenter:");  data2stream(data,stream,3);
				fprintf(stream,"\n\tR^2:\t%f",data[3]); 
				fprintf(stream,"\n\tColor:");  data2stream(element->color,stream,3);
				break;
			default: 
				break;
		}
	}
	return 0;
}

char* create_img_data(unsigned int width,unsigned int height,int comp){
	return (char*)malloc(width*height*comp);
}

//~ int save_img(char* data, const char* filepath,unsigned int width, unsigned int height, int comp){
	//~ return stbi_write_png (filepath, width, height,comp, data, width*comp);
//~ }