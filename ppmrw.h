#include <stdio.h>

typedef struct {
	unsigned char r,g,b;
} Pixel;

typedef struct{
	int width, height, max_value;
	Pixel* buffer;
} Image;

void write_file(FILE*, Image*, int);
void read_file(FILE*, Image*);
