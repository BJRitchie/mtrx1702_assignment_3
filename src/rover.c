// make clean && make
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "../include/bitmap.h"
#include "../include/functions.h"

// TODO free memory

int main(int argc, char **argv) {
    // TODO need to account for errors
    // printf("opening file\n");

    if (argc < 3) {
        printf("Input error: unable to process data/commands.\n"); 
        return 0;
    }

    FILE *fptr = fopen(argv[2], "r+"); 

    dataNode * data_matrix[WIDTH][HEIGHT];
    unsigned char topo_matrix[WIDTH][HEIGHT];
    unsigned char terrain_matrix[WIDTH][HEIGHT]; 
    corruptNode * corrupt_list_head = NULL; 
    int scigoal_arrayX[BUFFERSIZE_LARGE];
    int scigoal_arrayY[BUFFERSIZE_LARGE];
    int rover_x, rover_y;
    int numgoals=1;

    if (fptr == NULL) {
        printf("Input error: unable to process data/commands.\n"); 
        return 0;
    }
    else {
        unsigned char buffer[2]; 
        fread(buffer, sizeof(unsigned char), 2*BYTE, fptr);

        rover_x = buffer[0];
        rover_y = buffer[1];
        // printf("x: %d\n", rover_x);
        // printf("y: %d\n", rover_y);
        
        int numcorrupt = 0, i=0; 
        // NOTE: matrix is inverted in the y-direction 
        for ( int y = 0 ; y < HEIGHT ; y++ ) { 
            for ( int x = 0 ; x < WIDTH ; x++ ) { 
                // make a matrix of structures and clean it, then produce matrices
                unsigned char info_byte;
                fread(&info_byte, sizeof(info_byte), BYTE, fptr);

                dataNode * info_packet; 
                info_packet = new_info_packet(info_byte); 
                
                // MAY WANT TO MAKE LINKED LIST MORE ENERGY EFFICIENT PROGRAM
                
                if (info_packet->corrupt == CORRUPT) {
                    corruptNode * new_node;
                    new_node = create_corruptNode(x, y); 
                    corrupt_list_head = append_corruptNode(corrupt_list_head, new_node);
                    // printf("Corrupt Node detected at: (%d, %d)\n", x, y);
                    numcorrupt++;
                }
                else { 
                    if (info_packet->rover_pos == TRUE) {
                        rover_x = x;
                        rover_y = y;
                        // printf("Rover location detected at (%d, %d)\n", x, y);
                    }
                    if ( (info_packet->scigoal == TRUE) && (info_packet->corrupt == NOT_CORRUPT) ) {
                        // printf("scigoal created at: %d %d\n", x, y);
                        scigoal_arrayX[i] = x;
                        scigoal_arrayY[i] = y;
                        i++; 
                        numgoals++; 
                    }
                }
                data_matrix[y][x] = info_packet;
            }
        }

        clean(data_matrix, corrupt_list_head);
        make_array(data_matrix, topo_matrix, "height");
        make_array(data_matrix, terrain_matrix, "terrain");
        // save_map_bmp(topo_matrix, terrain_matrix, NULL, rover_x, rover_y, 
        // numgoals, scigoal_arrayX, scigoal_arrayY, "map_template/map_template.bmp");
    }

    fclose(fptr);

    if (strcmp(argv[1], "m") == SAME) {
        if (argc != 5) {
            printf("Input error: unable to process data/commands.\n"); 
            return 0;
        }
        
        int x = atoi(argv[3]);
        int y = atoi(argv[4]); 

        if ( ((x < WIDTH) && (x >= 0)) && (y < HEIGHT) && (y >= 0) ) {
            printf("Grid cell (x=%d, y=%d)\nTopo height: %dm\nTerrain type: %d\n", x, y, topo_matrix[y][x], terrain_matrix[y][x]);
        
            if ((data_matrix[y][x]->scigoal == TRUE) && (data_matrix[y][x]->corrupt == NOT_CORRUPT)) {
                printf("Specified location is a science goal\n");
            }
            if ((data_matrix[y][x]->rover_pos == TRUE)  && (data_matrix[y][x]->corrupt == NOT_CORRUPT)) {
                printf("Specified location is initial rover position\n");
            }
        }
        else {
            printf("Input error: unable to process data/commands.\n");
            return 0;
        }

        // free_datamatrix(data_matrix);
        return 0;
    }
    else if (strcmp(argv[1], "c") == SAME) {
        if (argc != 3) {
            printf("Input error: unable to process data/commands.\n"); 
            return 0;
        }

        char * result; 
        char line[BUFFERSIZE_LARGE]; 
        int rover_direction = NORTH;
        int energy = 0;
        result = fgets(line, BUFFERSIZE, stdin); 

        while ( strcmp(result, "end\n") != SAME ) { 
            char * motion;
            char * amount_tok;
            int amount, pass;
            
            motion = strtok(result, " ");
            amount_tok = strtok(NULL, "\n");
            amount = atoi(amount_tok); 

            // printf("recorded motion: %s, amount: (%d)\n", motion, amount);
            if ((strcmp(motion, "forward") == SAME) || (strcmp(motion, "backwards") == SAME)) {
                for (int i = 0; i < amount; i++) {
                    pass = make_move(data_matrix, &rover_x, &rover_y, motion, &rover_direction, &energy);
                    // printf("roverx: %d, rovery: %d\n", rover_x, rover_y);
                    
                    if (pass == FAIL) {
                        free_datamatrix(data_matrix); 
                        return 0;
                    }
                }
            }
            else if ( (strcmp(motion, "right") == SAME) || (strcmp(motion, "left") == SAME) ) {
                int turns = amount/90;
                for (int i = 0; i < turns; i++) {
                    pass = make_move(data_matrix, &rover_x, &rover_y, motion, &rover_direction, &energy);
                    // printf("roverx: %d, rovery: %d\n", rover_x, rover_y);
                    
                    if (pass == FAIL) {
                        free_datamatrix(data_matrix);
                        return 0;
                    }
                }
            }
            result = fgets(line, BUFFERSIZE, stdin); 
        }

        printf("Path is feasible\nEnergy required to complete path: %d\n", energy);
        free_datamatrix(data_matrix);
        return 0;
    }
    else if (strcmp(argv[1], "f") == SAME) {
        if (argc != 4) {
            printf("Input error: unable to process data/commands.\n"); 
            return 0;
        }

        int index = atoi(argv[3]);

        if ( index < numgoals ) {
            int path; 
            int scigoal_X = scigoal_arrayX[index];
            int scigoal_Y = scigoal_arrayY[index];
            // printf("scigoal_X: %d, scigoal_Y: %d\n", scigoal_X, scigoal_Y);
            // printf("height: %d\n", data_matrix[24][71]->height);
            corruptNode * goto_head;
            corruptNode * been[BUFFERSIZE_XLARGE];

            path = dfs(data_matrix, goto_head, been, rover_x, rover_y, scigoal_X, scigoal_Y);
            // printf("path: %d\n", path);
            if (path == PASS) { 
                printf("Feasible path exists.\n"); 
                free_been(been);
                free_datamatrix(data_matrix); 
            }
            else {
                printf("There are no feasible paths to this location.\n");
                free_been(been);
                free_datamatrix(data_matrix); 
            }
            return 0;
        }
        else {
            printf("Input error: unable to process data/commands.\n");
            return 0;
        }
        // printf("index: %d\n", index);
    }
    else {
        printf("Input error: unable to process data/commands.\n");
        return 0;
    }
    return 0;
}
