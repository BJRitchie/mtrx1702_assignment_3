#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "../include/bitmap.h"

#define BMP_HEADER_SIZE 0x36 // Assuming windows format
#define SIZE_OFFSET 0x02
#define PIXEL_ARRAY_OFFSET 0x0A
#define WIDTH_OFFSET 0x12
#define HEIGHT_OFFSET 0x16
#define PIXEL_SIZE_OFFSET 0x1C
#define DATA_SIZE_OFFSET 0x22

typedef struct {
    uint32_t file_size;
    uint32_t pixel_array_offset;    
    uint32_t height;
    uint32_t width;
    uint16_t pixel_size;
    uint32_t row_size;
    uint32_t data_size;

    uint8_t *raw;
} BmpHeader;

void check_fp(FILE *fp, char *filename) {
    if(fp == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        exit(1);
    }
}

void assert_file_format(bool condition) {
    if (!condition) {
        fprintf(stderr, "File format error\n");
        exit(1);
    }
}

Bmp read_bmp(char *filename) {

    FILE *fp = fopen(filename, "r");
    check_fp(fp, filename);

    // Struct to return results
    Bmp bmp;
    bmp.header = malloc(sizeof(BmpHeader));
    BmpHeader *header = bmp.header;

    // Read in standard header
    uint8_t standard_header[BMP_HEADER_SIZE];
    size_t bytes_read = fread(standard_header, 1, BMP_HEADER_SIZE, fp);
    assert_file_format(bytes_read == BMP_HEADER_SIZE);

    // Check file type
    assert_file_format(standard_header[0] == 'B' && standard_header[1] == 'M');
    //printf("%x\t%x\n", standard_header, *(uint32_t *)(standard_header + SIZE_OFFSET));

    header->file_size = *((uint32_t *)(standard_header + SIZE_OFFSET));
    header->pixel_array_offset =  *((uint32_t *)(standard_header + PIXEL_ARRAY_OFFSET));

    header->pixel_size = *((uint16_t *)(standard_header + PIXEL_SIZE_OFFSET)); // Pi
    assert_file_format(header->pixel_size == 24);

    header->width =  *((uint32_t *)(standard_header + WIDTH_OFFSET));
    header->height =  *((uint32_t *)(standard_header + HEIGHT_OFFSET));

    header->row_size = ((header->pixel_size * header->width + 31) / 32) * 4;

    #ifdef DEBUG
    printf("Row size %u\n",header->row_size);
    #endif

    header->data_size = *((uint32_t *)(standard_header + 0x22));

    // Read in entire header (everything but pixel array)
    rewind(fp);
    header->raw = malloc(sizeof(unsigned char) * header->pixel_array_offset);
    assert_file_format(header->raw != NULL);
    bytes_read = fread(header->raw, 1, header->pixel_array_offset, fp);

    // Read in rest of file
    char *raw_image = malloc(header->data_size);
    bytes_read = fread(raw_image, 1, header->data_size, fp);
    assert_file_format(bytes_read == header->data_size);

    // Allocate columns
    bmp.pixels = malloc(header->height * sizeof(unsigned char **));
    for (int i = 0; i < header->height; i++) {

        // Allocate rows
        bmp.pixels[i] = malloc(header->width * sizeof(unsigned char *));
        assert_file_format(bmp.pixels[i] != NULL);

        for (int j = 0; j < header->width; j++) {

            // Allocate pixels
            bmp.pixels[i][j] = malloc(3 * sizeof(unsigned char)); 
            assert_file_format(bmp.pixels[i][j] != NULL);
        }
    }

    // Read in each pixel
    for (int y = 0; y < header->height; y++) {
        for (int x = 0; x < header->width; x++) {
            
            // Read in each pixel
            bmp.pixels[y][x][BLUE] = *((unsigned char *)(raw_image + y * header->row_size + header->pixel_size/8 * x + 0));
            bmp.pixels[y][x][GREEN] = *((unsigned char *)(raw_image + y * header->row_size + header->pixel_size/8 * x + 1));
            bmp.pixels[y][x][RED] = *((unsigned char *)(raw_image + y * header->row_size + header->pixel_size/8 * x + 2));
        }
    }

    free(raw_image);
    fclose(fp);
    assert_file_format(header->data_size + header->pixel_array_offset == header->file_size);

    // Write height and width inside output bmp wrapper
    bmp.height = header->height;
    bmp.width = header->width;


    return bmp;
}


void assert_write(bool condition) {
    if (!condition) {
        fprintf(stderr, "file write error\n");
        exit(1);
    }
}


void write_bmp(Bmp bmp, char *filename) {

    FILE *fp = fopen(filename, "w");
    check_fp(fp, filename);

    BmpHeader *header = (BmpHeader *)bmp.header;

    // Write entire header (everything but pixel array)
    size_t bytes_written = fwrite(header->raw, 1, header->pixel_array_offset, fp);
    assert_write(bytes_written == header->pixel_array_offset);

    // Write rest of file
    // Loop backward through rows (image indexed from bottom left
    for (int y = 0; y < header->height; y++) {
        for (int x = 0; x < header->width; x++) {

            unsigned char *pixel = bmp.pixels[y][x];
            fwrite(&pixel[BLUE], 1, 1, fp);
            fwrite(&pixel[GREEN], 1, 1, fp);
            fwrite(&pixel[RED], 1, 1, fp);
        }

        // Write padding
        if ((header->width * 3) % 4 != 0) {
            char null_byte = 0x00;
            for (int i = 0; i < 4 - (header->width * 3) % 4; i++) {
                fwrite(&null_byte, 1, 1, fp);
            }
        }
    }

    fclose(fp);
}


void assert_copy(bool condition) {
    if (!condition) {
        fprintf(stderr, "file write error\n");
        exit(1);
    }
}


// Copy a bmp image
Bmp copy_bmp(Bmp old_bmp) {

    BmpHeader *old_header = (BmpHeader *)old_bmp.header;

    // Copy struct
    Bmp new_bmp = old_bmp;
    new_bmp.header = NULL;
    new_bmp.pixels = NULL;

    // Copy header
    BmpHeader *header = malloc(sizeof(BmpHeader));
    assert_copy(header != NULL);
    memcpy(header, old_header, sizeof(BmpHeader));
    new_bmp.header = header;
    header->raw = NULL;

    // Copy raw header
    header->raw = malloc(sizeof(unsigned char) * old_header->pixel_array_offset);
    assert_copy(header->raw != NULL);
    memcpy(header->raw, old_header->raw, old_header->pixel_array_offset);

    // Copy rest of image
    new_bmp.pixels = calloc(header->height, sizeof(unsigned char **));
    assert_copy(new_bmp.pixels != NULL);
    for (int i = 0; i < header->height; i++) {
        new_bmp.pixels[i] = calloc(header->width, sizeof(unsigned char *));
        assert_copy(new_bmp.pixels[i] != NULL);

        for (int j = 0; j < header->width; j++) {
            new_bmp.pixels[i][j] = malloc(3 * sizeof(unsigned char)); 
            assert_copy(new_bmp.pixels[i][j] != NULL);
            memcpy(new_bmp.pixels[i][j], old_bmp.pixels[i][j], 3 * sizeof(unsigned char));
        }
    }

    return new_bmp;
}

void free_bmp(Bmp bmp) {

    BmpHeader *header = (BmpHeader *)bmp.header;

    // Free each row
    for (int i = 0; i < header->height; i++) {

        // Free each pixel
        for (int j = 0; j < header->width; j++) {
            free(bmp.pixels[i][j]); 
            bmp.pixels[i][j] = NULL;
        }
        free(bmp.pixels[i]);
        bmp.pixels[i] = NULL;
    }
    free(bmp.pixels); 
    bmp.pixels = NULL;

    // Free raw header
    if (header != NULL) {
        free(header->raw);
        header->raw = NULL;
        free(header);
    }
}

static unsigned char topo_colours[8][3] = {
    {0,0,0},
    {32,16,0},
    {64,32,0},
    {96,48,0},
    {128,64,0},
    {160,80,0},
    {192,96,0},
    {224,112,0},
};

static unsigned char type_colours[4][3] = {
    {100,100,0},
    {100,50,0},
    {75,25,0},
    {255,0,0},
};

static void setcolour(unsigned char *pixel, unsigned char *colour) {
    pixel[0] = colour[0];
    pixel[1] = colour[1];
    pixel[2] = colour[2];
}

static unsigned char DANGER_COLOUR[] = {255,0,0};
static unsigned char PATH_COLOUR[] = {100,255,100};
static unsigned char ROVER_COLOUR[] = {255,255,255};
static unsigned char GOAL_COLOUR[] = {100,100,255};

// save_map_bmp: output an image of the map as BMP
void save_map_bmp(unsigned char topography[128][128], unsigned char terrain_type[128][128], 
    unsigned char **path_occupancy, 
    int roverx, int rovery,
    int ngoals, int goals_x[100], int goals_y[100], 
    char *filename) {

    // printf("save_map_bmp function activated\n");

    int offx = 32, offy = 24, offx2 = 320+32;
    int w = 128, h = 128;

    Bmp image = read_bmp("map_template/map_template.bmp");

    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            // printf("in for loop\n");

            int elevation = topography[r][c];
            int type = terrain_type[r][c];

            // printf("elevation and terrain type assigned\n");

            // Draw pixels for topography and terrain type
            // printf("Draw pixels for topography and terrain type\n");
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    // printf("drawing pixels for topography and terrain\n");
                    setcolour(image.pixels[2*r+offy+i][2*c+offx+j], topo_colours[elevation]);
                    setcolour(image.pixels[2*r+offy+i][2*c+offx2+j], type_colours[type]);
                }
            }

            // Draw pixels to indicate non-traversability, based on high slope
            // printf("Draw pixels to indicate non-traversability, based on high slope\n");
            if (r < (h-1)) {
                int slope = fabs(topography[r][c] - topography[r+1][c]);
                if (slope > 1) {
                    setcolour(image.pixels[2*r+offy+1][2*c+offx2], DANGER_COLOUR);
                    setcolour(image.pixels[2*r+offy+1][2*c+offx2+1], DANGER_COLOUR);
                }
            }
            if (c < (w-1)) {
                int slope = fabs(topography[r][c] - topography[r][c+1]);
                if (slope > 1) {
                    setcolour(image.pixels[2*r+offy][2*c+offx2+1], DANGER_COLOUR);
                    setcolour(image.pixels[2*r+offy+1][2*c+offx2+1], DANGER_COLOUR);
                }
            }

            // printf("Draw if proposed path covers this grid cell\n");
            // Draw if proposed path covers this grid cell
            if (path_occupancy != NULL && path_occupancy[r][c] == 1) {
                for (int i = 0; i < 2; i++) {
                    for (int j = 0; j < 2; j++) {
                        setcolour(image.pixels[2*r+offy+i][2*c+offx+j], PATH_COLOUR);
                        setcolour(image.pixels[2*r+offy+i][2*c+offx2+j], PATH_COLOUR);
                    }
                }
            }
        }
    }
    // printf("Draw rover\n"); 
    // Draw rover 
    if ( (roverx >= 0) && (rovery >= 0) && (roverx < 128) && (rovery < 128) ) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                setcolour(image.pixels[2*rovery+offy+i][2*roverx+offx+j], ROVER_COLOUR);
                setcolour(image.pixels[2*rovery+offy+i][2*roverx+offx2+j], ROVER_COLOUR);
            }
        }
    }
    // printf("Draw Science objectives\n");
    // Draw Science objectives
    for (int k = 0; k < ngoals; k++) {
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                int x = goals_x[k];
                int y = goals_y[k];
                setcolour(image.pixels[2*y+offy+i][2*x+offx+j], GOAL_COLOUR);
                setcolour(image.pixels[2*y+offy+i][2*x+offx2+j], GOAL_COLOUR);
            }
        }
    }

    write_bmp(image, filename);
    free_bmp(image);
}
