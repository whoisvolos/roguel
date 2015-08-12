#include <iostream>
#include <ncurses.h>
#include "maze.h"

using namespace std;

int COLS = 500, ROWS = 250;

int main() {
    /*
    initscr();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, ROWS, COLS);

    printw("%ix%i\n", COLS, ROWS);
    refresh();

    getch();

    endwin();
    */
    maze_t * maze = init_maze(COLS, ROWS);

    if (fill_maze(maze) != 0) {
        cerr << "Can not fill the maze" << endl;
        destroy_maze(maze);
        exit(-1);
    }

    /*
    char display_line[COLS + 1];
    for (int row = 0; row < ROWS; ++row) {
        maze_cell_t *line = get_line(maze, row);
        display_line[COLS] = 0;
        for (int col = 0; col < COLS; ++col) {
            maze_cell_t cell = *(line + col);
            display_line[col] = cell.is_floor ? (cell.flood_idx == 13 ? '.' : ' ') : '#';
            //display_line[col] = cell.is_floor ? (cell.flood_idx >= 0 ? (char)(cell.flood_idx + 48) : '.') : '#';
            //display_line[col] = cell.is_floor ? '.' : '#';
        }
        printf("%s\n", display_line);
    }
    */

    destroy_maze(maze);
    return 0;
}