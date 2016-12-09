#include <stdio.h>
#include <stdlib.h>
#include "ppmrw.h"

int input_type;
int output_type;

/*
	Used to peek at next char within file. 
	fh: File handle
	c: point to store char
*/
char* peek(FILE* fh, char* c){
	// Pulls first char, stores it, then puts it back.
	*c = fgetc(fh);
	ungetc(*c,fh);
	return c;
}

/*
	Checks if there is comment, if so it skips until a new line. 
	fh: File handle
*/
void skip_comments(FILE* fh){
	char c = 'a';
	// Check next char.
	peek(fh, &c);
	if (c == '#'){
		// Skips until finds new line.
		while (c = fgetc(fh), c != '\n'){}
	}
}

/*
	Reads an int from file.
	fh: File handle
*/
int read_value_from_header(FILE* fh, int i){
	// Checks if next value is comment and skips it.
	skip_comments(fh);
	fscanf(fh,"%i",&i);
	return i;
}

/*
	Reads a char while checking for end of file.
	fh: File handle
*/
char read_char(FILE* fh){
	char a = ' ';
	if (peek(fh,&a), a == -1){
		fprintf(stderr, "Error: Invalid P3 format.\n");
		exit(1);
	}
	fscanf(fh, "%i",(int*) (&a));
	return a;
}

/*
	Reads in a type 3 PPM file to buffer.
	fh: file handle
	buffer: buffer to store file data. 
*/
void read_type_3(FILE* fh, Image* img){
	int *a = malloc(sizeof(int));
	unsigned int i;
	for (i = 0; i < img->height * img->width; i += 1){
		// Changes data from binary to Pixel structure format
		Pixel pix;
		pix.r = read_char(fh);
		pix.g = read_char(fh);
		pix.b = read_char(fh);
		img->buffer[i] = pix;
	}
}

/*
	Reads in a type 6 PPM file to buffer.
	fh: file handle
	buffer: buffer to store file data. 
*/
void read_type_6(FILE* fh, Image *img){
	// Creates a sub buffer to read in all file data
	unsigned char* sub_buffer = malloc(sizeof(Pixel) * img->width * img->height);

	// Reading in file data & check if all data was read in
	if (fread(sub_buffer, 1,img->width * img->height * sizeof(Pixel) + 10, fh) != (img->width * img->height * sizeof(Pixel))){
		fprintf(stderr, "Error: Invalid P6 format.\n");
		exit(1);
	}
	
	// Goes through buffer and extracts the data in chunks
	unsigned int i;
	for (i = 0; i < img->width * img->height; i +=1){
		Pixel pix;
		pix.r = sub_buffer[i * 3];
		pix.g = sub_buffer[(i * 3) + 1];
		pix.b = sub_buffer[(i * 3) + 2];
		img->buffer[i] = pix;
	}
	
	// Frees buffer
	free(sub_buffer);
}

/*
	Reads in an image file to the Image structure
	fh: file handle
	img: Image structure to store image data
*/
void read_file(FILE* fh, Image* img){	
	char c;
	c = fgetc(fh);
	// Checks if magic number begins with P.
	if (c != 'P'){
		fprintf(stderr,"Invalid image file, first magic number: %c\n",c);
		exit(1);
	}
	
	// Check if appropiate file formats
	c = fgetc(fh);
	input_type = atoi(&c);
	fgetc(fh);
	
	// Reads in metadata.
	int i;
	img->width = read_value_from_header(fh, i);
	img->height = read_value_from_header(fh, i);
	img->max_value = read_value_from_header(fh, i);
	
	// Removes extra new line
	fgetc(fh);
	// Error checking
	if (img->width < 1){
		fprintf(stderr, "Invalid image width metadata\n");
		exit(1);
	} else if (img->height < 1){
		fprintf(stderr, "Invalid image height metadata\n");
		exit(1);
		
	} else if (img->max_value < 1 || img->max_value > 255){
		fprintf(stderr, "Invalid image max value metadata\n");
		exit(1);
	}
	
	
	// Create buffer to store photo
	img->buffer = malloc(sizeof(Pixel) * (img->width * img->height));
	// Choose method to read in data
	switch(input_type){
		case 3:
			read_type_3(fh, img);
			return;
		case 6:
			read_type_6(fh, img);
			return;
		default:
			fprintf(stderr,"Invalid PPM file, PPM file type not supported. %c\n",c);
			exit(1);
	}
}

/*
	Writes a P3 image from an Image structure to an file
	fh: file handle
	img: Image structure storing image data
*/
void write_type_3(FILE* fh, Image* img){
	unsigned int i;
	for (i = 0; i < img->height * img->width; i += 1){
		// Changes data from Pixel structure format to binary
		Pixel pix = img->buffer[i];
		fprintf(fh, "%i\n%i\n%i\n", pix.r, pix.g, pix.b);
	}
}

/*
	Writes a P6 image from an Image structure to an file
	fh: file handle
	img: Image structure storing image data
*/
void write_type_6(FILE* fh, Image* img){
	// Creates a sub_buffer to write all data to before file
	unsigned char *sub_buffer = malloc(sizeof(Pixel) * img->width * img->height);
	
	// Reading in file data
	for (int i = 0; i < img->width * img->height != 0; i+=1){	
		Pixel pix = img->buffer[i];
		sub_buffer[(i * 3)] = pix.r;
		sub_buffer[(i * 3) + 1] = pix.g;
		sub_buffer[(i * 3) + 2] = pix.b;
	}
	// Writes sub_buffer to file
	fwrite(sub_buffer, 1,img->width * img->height * sizeof(Pixel), fh);
	
	// Frees buffer
	free(sub_buffer);
}


/* 
	Writes image structure into output file
	fh: output file handle
	img: image structure to write
	output_type: the type of file to export as
*/
void write_file(FILE* fh, Image* img, int output_type){
	// writes out header to file stream
	fprintf(fh, "P%i\n%i %i\n%i\n", output_type, img->width, img->height, img->max_value);
	switch(output_type){
		case 3:
			write_type_3(fh, img);
			return;
		case 6:
			write_type_6(fh, img);
			return;
		default:
			fprintf(stderr,"Invalid PPM output type.\n");
			exit(1);
		
	}
}
