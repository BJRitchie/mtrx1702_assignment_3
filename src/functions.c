#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../include/functions.h"


dataNode * create_node(int height, int terrain, int scigoal, int rover_pos, int corrupt) {
    // creates a new node on the heap
    dataNode *new_node = (dataNode*)malloc(sizeof(dataNode));
    
    new_node->height = height;
    new_node->terrain = terrain;
    new_node->scigoal = scigoal;
    new_node->rover_pos = rover_pos;
    new_node->corrupt = corrupt;

    return new_node;
}

int getbits(unsigned char byte, int lowest_bit_index, int numbit) {
    // grabs bits from lowest_bit_index to lowest_bit_index + numbits 
    return ((byte >> lowest_bit_index) & ((1 << numbit) - 1)); 
}

int parity_check(unsigned char byte) {
    int parity = 0;

    while (byte) {
        parity = !(parity);
        byte = byte & (byte-1);
    }
    return parity;
}

dataNode * new_info_packet(unsigned char info_byte) {
    // creates a new node on the heap
    dataNode *new_node = (dataNode*)malloc(sizeof(dataNode));
    
    int height, terrain, scigoal, rover_pos, parity;
    height = getbits(info_byte, 5, 3);
    terrain = getbits(info_byte, 3, 2);
    scigoal = getbits(info_byte, 2, 1);
    rover_pos = getbits(info_byte, 1, 1);
    parity = getbits(info_byte, 0, 1);

    new_node->height = height;
    new_node->terrain = terrain;
    new_node->scigoal = scigoal;
    new_node->rover_pos = rover_pos;
    
    parity = parity_check(info_byte);
    // if there's an even number of 1s, par will be 0
    // the parity bit should make the number of 1s even for an uncorrupted byte
    if ( parity == 1 ) {
        new_node->corrupt = CORRUPT;
    }
    else if ( parity == 0 ) {
        new_node->corrupt = NOT_CORRUPT;
    }
    return new_node;
}

int most_common(int * array, int array_length) {
    // finds the most common (non-corrupt) element in an array, returning lowest val if multiple
    int max_count = 0, index = 0;
    for (int i = 0; i < array_length; i++) {
        // printf("array[%d] = %d\n", i, array[i]);
        int count = 0;
        if (array[i] != -1) { 
            for (int j = 0; j < array_length; j++) {
                if ( array[i] == array[j] ) {
                    count++;
                }
            }
            if ((max_count < count) || ( (max_count == count) && (array[i] < array[index]))) {
                max_count = count;
                index = i;
            }
        }
        // printf("count: %d\n", count);
    }
    // printf("returning: %d\n", array[index]);
    return array[index]; 
}

void create_array(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * current, int * array, char * type) {
    int count=0;

    if ( strcmp(type, "terrain") == SAME ) {
        for (int j = -1; j < 2; j++) {
            for (int i = -1; i < 2; i++) {
                if (data_matrix[current->y+j][current->x+i]->corrupt == CORRUPT) {
                    array[count] = -1;
                    count++;
                }
                else {
                    array[count] = data_matrix[current->y+j][current->x+i]->terrain;
                    count++;
                }
            }
        }
    }
    else if ( strcmp(type, "height") == SAME ) {
        for (int j = -1; j < 2; j++) {
            for (int i = -1; i < 2; i++) {
                if (data_matrix[current->y+j][current->x+i]->corrupt == CORRUPT) {
                    array[count] = -1;
                }
                else {
                    array[count] = data_matrix[current->y+j][current->x+i]->height;
                }
                count++;
            }
        }
    }
    else {
        printf("error in create_array - type not recognised\n");
    }
    return;
}

void clean(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * corrupt_list_head) {
    // need to clean data that has been flagged as corrupt 
    corruptNode * current = corrupt_list_head; 

    while ( current != NULL ) { 
        // corrupt input is data_matrix[current->y][current->x] 
        // want to look at surrounding values to find the replacement height and terrain
        
        // most commonly occuring terrain value

        // make array of values around the index
        int terr_array[9];
        create_array(data_matrix , current, terr_array, "terrain");
        
        int most_frequent_element;
        most_frequent_element = most_common(terr_array, 9);
        data_matrix[current->y][current->x]->terrain = most_frequent_element;

        // most commonly occuring terrain value
        int top_array[9];
        create_array(data_matrix , current, top_array, "height");

        most_frequent_element = most_common(top_array, 9);
        data_matrix[current->y][current->x]->height = most_frequent_element;

        current = current->next;
    }
    return;
}

corruptNode * create_corruptNode(int x, int y) {
    
    corruptNode *new_node = (corruptNode*)malloc(sizeof(corruptNode));
    new_node->x = x;
    new_node->y = y;
    new_node->next = NULL;

    return new_node;
}

corruptNode * append_corruptNode(corruptNode *head, corruptNode *new_node) {
    // adds node to the end

    if (head == NULL) {
        // needs new head
        return new_node; // the new head 
    }

    corruptNode * current = head;
    while (current->next != NULL) {
        // traverses to the end of the list
        current = current->next;
    }
    current->next = new_node;
    return head;
}

void make_array (dataNode * data_matrix[WIDTH][HEIGHT], unsigned char matrix[WIDTH][HEIGHT], char * type) {
    if ( strcmp(type, "terrain") == SAME ) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                matrix[y][x] = (unsigned char)data_matrix[y][x]->terrain;
            }
        }
        return;
    }
    else if ( strcmp(type, "height") == SAME ) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                matrix[y][x] = (unsigned char)data_matrix[y][x]->height;
            }
        }
        return;
    }
    else {
        printf("error in make_array - type not recognised");
    }
}

void print_arr(dataNode * data_matrix[WIDTH][HEIGHT]) {
    int i = 1;
    for (int y=0; y < 2; y++) {
        for (int x=0; x < 2; x++) {
            printf("i: %d\n", i);
            i++;
            dataNode * node = data_matrix[y][x];
            printf("height: %d\nterrain: %d\nscigoal: %d\nrover_pos: %d\ncorrupt: %d\n---------\n", 
            node->height, node->terrain, node->scigoal, node->rover_pos, node->corrupt);
        }
    }
}

void print_maps(unsigned char topography[128][128], unsigned char terrain_type[128][128]) {
    int h = 128, w = 128;
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            int elevation = topography[r][c];   
            int type = terrain_type[r][c];
            printf("elev: %i, type: %i\n", elevation, type);
        }
    }
}

int make_move( dataNode * data_matrix[WIDTH][HEIGHT] ,int * rover_x, 
int * rover_y, char * motion, int * rover_direction, int * energy) 
{
    // probably the worst function I've ever written
    int terrain_next;
    int slope, height_curr, height_next;
    int direction = *rover_direction;
    // check for index moving out of bounds
    if ( strcmp(motion, "forward") == SAME ) {
        switch (direction) {
            case NORTH : {
                if ( ((*rover_y+1) < 128 )) {
                    terrain_next = data_matrix[*rover_y+1][*rover_x]->terrain;
                    height_curr = data_matrix[*rover_y][*rover_x]->height;
                    height_next = data_matrix[*rover_y+1][*rover_x]->height;
                    slope = abs(height_curr - height_next);

                    if ( (terrain_next != 3) && (slope < 1.5) ) {
                        *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                        (*rover_y)++;
                    }
                    else {
                        printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                        return -1; 
                    }
                }
                else {
                    printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                    return -1;
                }
                break;
            }

            case SOUTH: {
            if (((*rover_y-1) > 0 )) {
                terrain_next = data_matrix[*rover_y-1][*rover_x]->terrain;
                height_curr = data_matrix[*rover_y][*rover_x]->height;
                height_next = data_matrix[*rover_y-1][*rover_x]->height;
                slope = abs(height_curr - height_next);

                if ( (terrain_next != 3) && (slope < 1.5) ) {
                    *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                    (*rover_y)--;
                }
                else {
                    printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                    return -1;
                }
            }
            else {
                printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                return -1;
            }
            break;
            }

            case WEST: {
                if ( ((*rover_x-1) > 0 )) {
                    terrain_next = data_matrix[*rover_y][*rover_x-1]->terrain;
                    height_curr = data_matrix[*rover_y][*rover_x]->height;
                    height_next = data_matrix[*rover_y][*rover_x-1]->height;
                    slope = abs(height_curr - height_next);

                    if ( (terrain_next != 3) && (slope < 1.5) ) {
                        *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                        (*rover_x)--;
                    }
                    else {
                        printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                        return -1;
                    }
                }
                else {
                    printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                    return -1;
                }
                break;
            }

            case EAST: {
                    if ( ((*rover_x+1) < 128)) {
                        terrain_next = data_matrix[*rover_y][*rover_x+1]->terrain;
                        height_curr = data_matrix[*rover_y][*rover_x]->height;
                        height_next = data_matrix[*rover_y][*rover_x+1]->height;
                        slope = abs(height_curr - height_next);

                        if ( (terrain_next != 3) && (slope < 1.5) ) {
                            *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                            (*rover_x)++;
                        }
                        else {
                            printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                            return -1;
                        }
                    }
                    else {
                        printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                        return -1;
                    }
                break;
            }
            default:
                printf("failure in switch-case");
                return -1;
        }
        return 0;
    }
    else if ( strcmp(motion, "backwards") == SAME ) {
        switch (direction) {
            case NORTH: { 
                if (((*rover_y-1) > 0 )) {
                    terrain_next = data_matrix[*rover_y-1][*rover_x]->terrain;
                    height_curr = data_matrix[*rover_y][*rover_x]->height;
                    height_next = data_matrix[*rover_y-1][*rover_x]->height;
                    slope = abs(height_curr - height_next);

                    if ( (terrain_next != 3) && (slope < 1.5) ) {
                        *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                        (*rover_y)--;
                    }
                    else {
                        printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                        return -1;
                    }
                }
                else {
                    printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                    return -1;
                }
                break;
            }

            case SOUTH: {
            if (((*rover_y+1) < 128 )) {
                terrain_next = data_matrix[*rover_y+1][*rover_x]->terrain;
                height_curr = data_matrix[*rover_y][*rover_x]->height;
                height_next = data_matrix[*rover_y+1][*rover_x]->height;
                slope = abs(height_curr - height_next);

                if ( (terrain_next != 3) && (slope < 1.5) ) {
                    *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                    (*rover_y)++;
                }
                else {
                    printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                    return -1;
                }
            }
            else {
                printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                return -1;
            }
            break;
            }

            case WEST: {
                if (((*rover_x+1) < 128 )) {
                    terrain_next = data_matrix[*rover_y][*rover_x+1]->terrain;
                    height_curr = data_matrix[*rover_y][*rover_x]->height;
                    height_next = data_matrix[*rover_y][*rover_x+1]->height;
                    slope = abs(height_curr - height_next);

                    if ( (terrain_next != 3) && (slope < 1.5) ) {
                        *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                        (*rover_x)++;
                    }
                    else {
                        printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                        return -1;
                    }
                }
                else {
                    printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                    return -1;
                }
                break;
            }

            case EAST: {
                    if ( ((*rover_x-1) > 0 )) {
                        terrain_next = data_matrix[*rover_y][*rover_x-1]->terrain;
                        height_curr = data_matrix[*rover_y][*rover_x]->height;
                        height_next = data_matrix[*rover_y][*rover_x-1]->height;
                        slope = abs(height_curr - height_next);

                        if ( (terrain_next != 3) && (slope < 1.5) ) {
                            *energy = *energy + pow(2, terrain_next) + slope*CLIME_ENERGY;
                            (*rover_x)--;
                        }
                        else {
                            printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y); 
                            return -1;
                        }
                    }
                    else {
                        printf("Path is not feasible\nLast feasible position: (x=%d, y=%d)\n", *rover_x, *rover_y);
                        return -1;
                    }
                break;
            }

            default:
                printf("failure in switch-case");
                return -1;
        }
        return 0;
    }
    else if ( strcmp(motion, "right") == SAME ) {
        switch (direction) {
            case NORTH: {
                *rover_direction = EAST;
                break;
            }

            case SOUTH: {
                *rover_direction = WEST;
                break;
            }

            case EAST: {
                *rover_direction = SOUTH;
                break;
            }

            case WEST: {
                *rover_direction = NORTH;
                break;
            }
        }
        return 0;
    }
    else if ( strcmp(motion, "left") == SAME ) {
        switch (direction) {
            case NORTH: {
                *rover_direction = WEST;
                break;
            }

            case SOUTH: {
                *rover_direction = EAST;
                break;
            }

            case EAST: {
                *rover_direction = NORTH;
                break;
            }

            case WEST: {
                *rover_direction = SOUTH;
                break;
            }
        }
        return 0;
    }
    else {
        printf("command not recognised\n");
        return -1;
    }
}

int in_array(int x, int y, corruptNode * been[BUFFERSIZE_XLARGE], long int * been_index) {
    // printf("in_array activated\n");
    for (int i = 0; i < (*been_index); i++) {
        // printf("in_array; i: %d\n", i);
        if ( (been[i] != NULL) && ( been[i]->x == x ) && ( been[i]->y == y) ) {
            return TRUE;
        }
    }
    return FALSE;
}

int in_go(int x, int y, corruptNode * goto_head) {
    corruptNode * current = goto_head;

    while (current != NULL) {
        if ( ( current->x == x ) && ( current->y == y ) ){
            return TRUE;
        }
        current = current->next;
    }
    return FALSE;
}

corruptNode * dfs_next(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * goto_head, corruptNode * been[BUFFERSIZE_XLARGE], long int *been_index) {
    
    // printf("goto_head->next: %p\n", goto_head->next);
    
    long int goto_count = 0;
    int in_been, in_goto;
    int terrain_next=0, height_curr=0, height_next=0, slope=0;
    
    int x0 = goto_head->x;
    int y0 = goto_head->y;
    
    int movements[4][2] = {
        {(goto_head->x), ((goto_head->y)+1)}, // up
        {(goto_head->x), ((goto_head->y)-1)}, // down
        {((goto_head->x)-1), (goto_head->y)}, // left
        {((goto_head->x)+1), ((goto_head->y))}  // right
    };

    for (int i = 0; i < 4; i++) {
        // printf("x: %d, y: %d\n", movements[i][0], movements[i][1]);
        int x = movements[i][0];
        int y = movements[i][1];

        if ( ((x < WIDTH) && (x >= 0)) && (y < HEIGHT) && (y >= 0) ) {
            
            terrain_next = (data_matrix[y][x])->terrain;

            height_curr = (data_matrix[y0][x0])->height;
            height_next = (data_matrix[y][x])->height;
            slope = abs(height_next - height_curr);
            
            // printf("xtest: %d, ytest: %d\n", x, y);
            // printf("height_curr: %d\nheight_next: %d\n", height_curr, height_next);

            in_been = in_array(x, y, been, been_index); 
            in_goto = in_go(x, y, goto_head); 

            // printf("terrain_next: %d\nslope: %d\nin_been; %d\nin_goto: %d\n(----------)\n", terrain_next, slope, in_been, in_goto);

            if ( (slope < 1.5 ) && (terrain_next < 3) && (in_been != TRUE) && (in_goto != TRUE) ) {
                /* slope means it is accessible and terrain is traversible */ 
                // printf("appending node(dfs_next)\n");
                corruptNode * new_node = create_corruptNode(x, y);
                goto_head = append_corruptNode(goto_head, new_node);
                goto_count++;
                // printf("add\n");
                // printf("node appended(dfs_next)\n");
            }
        }
    }

    // printf("adding visited to been(dfs_next)\n");
    // printf("been_index: %d (dfs_next)\n", *been_index);

    // printf("x0: %d, y0: %d\n", x0, y0);

    been[*been_index] = goto_head; 
    (*been_index)++; 
    goto_head = goto_head->next; 
    // printf("goto_count: %d\n(----------)\n", goto_count);
    // printf("been_index: %li\n", *been_index);
    goto_count--;

    return goto_head;
}

int dfs(dataNode * data_matrix[WIDTH][HEIGHT], corruptNode * goto_head, corruptNode * been[BUFFERSIZE_XLARGE], int rover_x, int rover_y, int scigoal_x, int scigoal_y) 
{
    /* 
    starting point for the dfs is the rover
    if the system reaches the scigoal coordinates without breaking rules, 
    it is successful 
    */

    // can use corruptNode struct as it stores only the x and y values of desired array positions
    
    goto_head = create_corruptNode(rover_x, rover_y);
    long int been_index=0;
    // printf("variables assigned\n");
    // printf("roverx: %d, rovery: %d (dfs)\n", goto_head->x, goto_head->y);
    // printf("height: %d\n", data_matrix[24][74]->height);

    // int it=0;
    while (goto_head != NULL) {
    // while (it < 40) {
        if ((goto_head->x == scigoal_x) && (goto_head->y == scigoal_y)) {
            return PASS;
        }
        else {
            // printf("roverx: %d, rovery: %d (dfs)\n", goto_head->x, goto_head->y);
            goto_head = dfs_next(data_matrix, goto_head, been, &been_index);
        }
        // it++;
    }
    // free_been(been);
    // printf("while loop exited\n");
    return FAIL;
}

void free_been(corruptNode * been[BUFFERSIZE_XLARGE]) {
    for (int i = 0; i < BUFFERSIZE_XLARGE; i++) {
        free(been[i]);
    }
}

void free_datamatrix(dataNode * data_matrix[WIDTH][HEIGHT]) {
    for ( int y = 0 ; y < HEIGHT ; y++ ) { 
        for ( int x = 0 ; x < WIDTH ; x++ ) { 
            free(data_matrix[y][x]);
        }
    }
}