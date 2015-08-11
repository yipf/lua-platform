#ifndef IO_H
#define IO_H

#include <stdio.h>
#include "ray-tracer.h"

#include "stb_image.h"


scalar* fill_data(FILE* stream,scalar* dst,unsigned int length);

element_t append_element(element_t current,int type, scalar* data, scalar* color);

int stream2scene(FILE* stream,element_t element,element_t light,scalar* output_config, scalar* view);
int data2stream(scalar* data,FILE* stream,unsigned int length);
int scene2stream(element_t element,element_t light,FILE* stream);

char* create_img_data(unsigned int width,unsigned int height,int comp);
/* int save_img(char* data,const char* filepath,unsigned int width,unsigned int height,int comp);  */

#endif