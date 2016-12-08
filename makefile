SHELL = /bin/sh

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=gnu11 -Wwrite-strings
LDFLAGS = -lm -lglfw -lGL
OPTFLAG= -O3
OUTPUT = ezview 

SOURCES = $(wildcard ./*.c)

.PHONY: all clean build clear

all: build

build: 
	$(CC) $(SOURCES) -o $(OUTPUT) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OUTPUT) 
	rm -f $(OUTPUT).exe*
	rm -f $(OUTPUT_LOC)/*.ppm
	rm -f $(INPUT_LOC)/*~
	rm -f $(INPUT_LOC)/*#
	rm -f *.o
	rm -f *~
	rm -f *.stackdump
	rm -f *#

clear :
	clear

rebuild: clean build

retest: rebuild
	./$(OUTPUT)

restest: rebuild
	./$(OUTPUT) 5 5 $(MAIN_JSON) $(patsubst $(INPUT_LOC)/%.json,$(OUTPUT_LOC)/%.ppm, $(MAIN_JSON))

reftest: rebuild
	$(foreach file, $(INPUT_FILES), $(info $(shell ./$(OUTPUT) $(IMG_SIZE) $(file) $(patsubst $(INPUT_LOC)/%.json,$(OUTPUT_LOC)/%.ppm, $(file)))))

debug: clean
	$(CC) $(SOURCES) -o $(OUTPUT) $(CFLAGS) $(LDFLAGS) -g
	gdb ./$(OUTPUT)

optimize: clean 
	$(CC) $(SOURCES) -o $(OUTPUT) $(CFLAGS) $(LDFLAGS) $(OPTFLAG)
