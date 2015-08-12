//
// Created by whoisvolos on 10/08/15.
//

#ifndef PLAYGROUND_MAZE_H
#define PLAYGROUND_MAZE_H

#include <random>
#include <stdlib.h>
#include <vector>
#include <map>
#include "graph.h"

struct maze_cell_t {
    bool is_floor;
    int flood_idx;

    maze_cell_t(): is_floor(false), flood_idx(-1) {}
};

struct maze_t {
    int width;
    int height;
    maze_cell_t *cells;

    maze_t(int width, int height): width(width), height(height), cells(NULL) {}
};

maze_t* init_maze(int width, int height);
int fill_maze(maze_t *maze);
void destroy_maze(maze_t *maze);
maze_cell_t* get_line(maze_t* maze, int row);

#endif //PLAYGROUND_MAZE_H
