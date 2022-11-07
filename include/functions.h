#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define PASS 0
#define FAIL -1

#define SAME 0

#define BYTE 1 
#define CORRUPT 1
#define NOT_CORRUPT 0
#define WIDTH 128
#define HEIGHT 128

#define BUFFERSIZE 50
#define BUFFERSIZE_LARGE 100
#define BUFFERSIZE_XLARGE 1700

#define NORTH 1
#define EAST 2
#define SOUTH -1
#define WEST -2

// units of energy for the terrain types
#define TERR_0 1
#define TERR_1 2
#define TERR_2 4 

#define CLIME_ENERGY 10 // per m in height change

typedef struct data {
    unsigned char height;                         // m
    unsigned char terrain;                        // 0-3
    int scigoal, rover_pos, corrupt;    // TRUE or FALSE (1,0)
} dataNode; 

typedef struct corrupt {
    int x,y;
    struct corrupt *next;
} corruptNode;


dataNode * create_node(int height, int terrain, int scigoal, int rover_pos, int corrupt);

int getbits(unsigned char byte, int lowest_bit_index, int numbit);

int parity_check(unsigned char byte);

dataNode * new_info_packet(unsigned char info_byte);

void print_arr(dataNode * data_matrix[WIDTH][HEIGHT]);

void clean(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * corrupt_list_head);

corruptNode * create_corruptNode(int x, int y);

corruptNode * append_corruptNode(corruptNode *head, corruptNode *new_node);

int most_common(int * array, int array_length);

void create_array(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * current, int * array, char * type);

void make_array (dataNode * data_matrix[WIDTH][HEIGHT], unsigned char matrix[WIDTH][HEIGHT], char * type);

void print_maps(unsigned char topography[128][128], unsigned char terrain_type[128][128]);

int make_move( dataNode * data_matrix[WIDTH][HEIGHT] ,int * rover_x, 
int * rover_y, char * motion, int * rover_direction, int * energy);

int in_array(int x, int y, corruptNode * been[BUFFERSIZE_XLARGE], long int * been_index);

int in_go(int x, int y, corruptNode * goto_head);

corruptNode * dfs_next(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * goto_head, corruptNode * been[BUFFERSIZE_XLARGE], long int *been_index); 

int dfs(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * goto_head, corruptNode * been[BUFFERSIZE_XLARGE], int rover_x, int rover_y, int scigoal_x, int scigoal_y);

void free_been(corruptNode * been[BUFFERSIZE_XLARGE]);

void free_datamatrix(dataNode * data_matrix[WIDTH][HEIGHT]);

#endif