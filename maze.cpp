//
// Created by whoisvolos on 10/08/15.
//

#include "maze.h"

#define CUTOFF_WALL 5
#define NUM_SIM 10
#define CUTOFF_WALL_DIST 1
#define CUTOFF_EMPTY_DIST 3
#define CUTOFF_FLOOD_OPEN 2
#define WALL_PROB 50

static std::mt19937 rnd_engine(1000);

static int count_walls_around(maze_t* maze, int x, int y, int steps_around);
static int count_walls_around(maze_cell_t* cells, int width, int height, int x, int y, int steps_around);
static void make_border_walls(maze_cell_t *cells, int width, int height);

struct ff_cell_t {
    bool is_floor;
    bool flooded;
    int x;
    int y;
    int idx;
};
static inline void push_if_empty(std::vector<ff_cell_t *> &vec, std::vector<ff_cell_t *> &vec_tmp, ff_cell_t *cell);

struct cc_passageway {
    ff_cell_t* from;
    ff_cell_t* to;

    int get_dist() {
        if (!from || !to) {
            return INT32_MAX;
        } else if (from->x == to->x) {
            return abs(from->y - to->y);
        } else {
            return abs(from->x - to->x);
        }
    }
};

maze_t* init_maze(int width, int height) {
    maze_t* maze = new maze_t(width, height);
    maze->cells = new maze_cell_t[width * height];
    return maze;
}

int fill_maze(maze_t *maze) {
    // First random (uniform, p = 0.45) fill
    static std::uniform_int_distribution<int> perc_distr(0, 100);
    maze_cell_t* cells = maze->cells;
    for (int i = 0, l = maze->width * maze->height; i < l; ++i) {
        cells[i].is_floor = perc_distr(rnd_engine) >= WALL_PROB;
    }

    // Now make convolutions to create good looking dungeon
    for (int sim = 0; sim < NUM_SIM; ++sim) { ;
        // Create new cells
        maze_cell_t *cells_new = new maze_cell_t[maze->width * maze->height];
        // Make convolution
        int steps_around = 1, cutoff_wall = CUTOFF_WALL;
        for (int y = 0, ly = maze->height; y < ly; ++y) {
            for (int x = 0, lx = maze->width; x < lx; ++x) {
                int cnt_walls = count_walls_around(maze, x, y, CUTOFF_WALL_DIST);
                int is_open = count_walls_around(maze, x, y, CUTOFF_EMPTY_DIST) <= 1;
                cells_new[y * maze->width + x].is_floor = cnt_walls < cutoff_wall && !is_open;
            }
        }
        delete[] maze->cells;
        maze->cells = cells_new;
    }

    // Make border walls
    make_border_walls(maze->cells, maze->width, maze->height);

    // Flood check
    // Fill flooded cells
    ff_cell_t ffill[maze->height][maze->width];
    int cnt = 0;
    for (int y = 0, ly = maze->height; y < ly; ++y) {
        for (int x = 0, lx = maze->width; x < lx; ++x) {
            maze_cell_t *cur_cell = maze->cells + cnt;
            ff_cell_t *cur_f_cell = ffill[y] + x;
            cur_f_cell->x = x;
            cur_f_cell->y = y;
            cur_f_cell->flooded = false;
            cur_f_cell->is_floor = cur_cell->is_floor;
            cur_f_cell->idx = -1;
            cnt++;
        }
    }

    // Mark flooded cells by cave index
    int cur_idx = -1;
    std::map<int, std::vector<ff_cell_t *> > caves;
    while(true) {
        cur_idx++;
        //printf("Flooding for index %i\n", cur_idx);
        bool not_flooded_found = false;
        std::vector<ff_cell_t*> vec;
        std::vector<ff_cell_t *> tmp_vec;
        for (int y = 0, ly = maze->height; y < ly; ++y) {
            for (int x = 0, lx = maze->width; x < lx; ++x) {
                ff_cell_t *cur_f_cell = ffill[y] + x;
                if (!cur_f_cell->is_floor || cur_f_cell->flooded) {
                    continue;
                }
                not_flooded_found = true;
                //printf("Found not flooded cell @ (%i, %i)\n", cur_f_cell->x, cur_f_cell->y);
                vec.push_back(cur_f_cell);
                tmp_vec.push_back(cur_f_cell);
                break;
            }
            if (not_flooded_found) {
                break;
            }
        }
        if (not_flooded_found) {
            while (!vec.empty()) {
                ff_cell_t *cur_cell = vec.back();
                vec.pop_back();
                int x = cur_cell->x;
                int y = cur_cell->y;
                cur_cell->flooded = true;
                cur_cell->idx = cur_idx;
                if (x < maze->width - 1) {
                    push_if_empty(vec, tmp_vec, ffill[y] + x + 1);
                }
                if (x > 0) {
                    push_if_empty(vec, tmp_vec, ffill[y] + x - 1);
                }
                if (y > 0) {
                    push_if_empty(vec, tmp_vec, ffill[y - 1] + x);
                }
                if (y < maze->height - 1) {
                    push_if_empty(vec, tmp_vec, ffill[y + 1] + x);
                }
            }
        } else {
            break;
        }
        caves.insert(std::pair<int, std::vector<ff_cell_t *> >(cur_idx, tmp_vec));
    }

    std::map<int, cc_passageway> cc_map;

    for (auto i : caves) {
        // right most cell
        int idx = i.first;
        int co_idx = -1;
        for (auto cell : i.second) {
            // Span to the right
            for (int x = cell->x + 1, lx = maze->width; x < lx; ++x) {
                ff_cell_t* co_cell = ffill[cell->y] + x;
                if (co_cell->is_floor) {
                    if (co_cell->idx != idx) {
                        int cc_key = (std::min(idx, co_cell->idx) << 16 & 0xFFFF0000) | (std::max(idx, co_cell->idx) & 0x0000FFFF);
                        cc_map.insert(std::pair<int, cc_passageway>(cc_key, { NULL, NULL }));
                        auto it = cc_map.find(cc_key);
                        if (x - (cell->x + 1) < it->second.get_dist()) {
                            it->second.from = cell;
                            it->second.to = co_cell;
                        }
                    }
                    break;
                }
            }
            // Span to the left
            for (int x = cell->x - 1; x >= 0; --x) {
                ff_cell_t* co_cell = ffill[cell->y] + x;
                if (co_cell->is_floor) {
                    if (co_cell->idx != idx) {
                        int cc_key = (std::min(idx, co_cell->idx) << 16 & 0xFFFF0000) | (std::max(idx, co_cell->idx) & 0x0000FFFF);
                        cc_map.insert(std::pair<int, cc_passageway>(cc_key, { NULL, NULL }));
                        auto it = cc_map.find(cc_key);
                        if (cell->x - x - 1 < it->second.get_dist()) {
                            it->second.from = cell;
                            it->second.to = co_cell;
                        }
                    }
                    break;
                }
            }
            // Span to the bottom
            for (int y = cell->y + 1, ly = maze->height; y < ly; ++y) {
                ff_cell_t* co_cell = ffill[y] + cell->x;
                if (co_cell->is_floor) {
                    if (co_cell->idx != idx) {
                        int cc_key = (std::min(idx, co_cell->idx) << 16 & 0xFFFF0000) | (std::max(idx, co_cell->idx) & 0x0000FFFF);
                        cc_map.insert(std::pair<int, cc_passageway>(cc_key, { NULL, NULL }));
                        auto it = cc_map.find(cc_key);
                        if (y - (cell->y + 1) < it->second.get_dist()) {
                            it->second.from = cell;
                            it->second.to = co_cell;
                        }
                    }
                    break;
                }
            }
            // Span to the top
            for (int y = cell->y - 1; y >= 0; --y) {
                ff_cell_t* co_cell = ffill[y] + cell->x;
                if (co_cell->is_floor) {
                    if (co_cell->idx != idx) {
                        int cc_key = (std::min(idx, co_cell->idx) << 16 & 0xFFFF0000) | (std::max(idx, co_cell->idx) & 0x0000FFFF);
                        cc_map.insert(std::pair<int, cc_passageway>(cc_key, { NULL, NULL }));
                        auto it = cc_map.find(cc_key);
                        if (cell->y - y - 1 < it->second.get_dist()) {
                            it->second.from = cell;
                            it->second.to = co_cell;
                        }
                    }
                    break;
                }
            }
        }
    }

    for (auto it : cc_map) {
        int from = (it.first >> 16) & 0x0000FFFF;
        int to = it.first & 0x0000FFFF;
        printf("(%i,%i): %i\n", from, to, it.second.get_dist());
    }

    cnt = 0;
    for (int y = 0, ly = maze->height; y < ly; ++y) {
        for (int x = 0, lx = maze->width; x < lx; ++x) {
            maze_cell_t *cur_cell = maze->cells + cnt;
            ff_cell_t *cur_f_cell = ffill[y] + x;
            if (cur_f_cell->flooded) {
                cur_cell->flood_idx = cur_f_cell->idx;
            }
            cnt++;
        }
    }

    return 0;
}

maze_cell_t* get_line(maze_t* maze, int row) {
    return maze->cells + row * maze->width;
}

void destroy_maze(maze_t *maze) {
    if (maze) {
        if (maze->cells) {
            delete[] maze->cells;
        }
        delete maze;
    }
}

int count_walls_around(maze_t* maze, int x, int y, int steps_around) {
    return count_walls_around(maze->cells, maze->width, maze->height, x, y, steps_around);
}

int count_walls_around(maze_cell_t* cells, int width, int height, int x, int y, int steps_around) {
    int x_min = std::max(0, x - steps_around);
    int x_max = std::min(x + steps_around + 1, width);
    int y_min = std::max(0, y - steps_around);
    int y_max = std::min(y + steps_around + 1, height);
    int result = 0;

    for (int y_curr = y_min; y_curr < y_max; ++y_curr) {
        int offs = y_curr * width;
        for (int x_curr = x_min; x_curr < x_max; ++x_curr) {
            result += cells[offs + x_curr].is_floor ? 0 : 1;
        }
    }

    return result;
}

void make_border_walls(maze_cell_t *cells, int width, int height) {
    int wall_hor_offset = (height - 1) * width;
    for (int i = 0, l = width; i < l; ++i) {
        cells[i].is_floor = false;
        cells[i + wall_hor_offset].is_floor = false;
    }
    for (int i = 1, l = height - 1; i < l; ++i) {
        cells[i * width].is_floor = false;
        cells[i * width + width - 1].is_floor = false;
    }
}

static inline void push_if_empty(std::vector<ff_cell_t *> &vec, std::vector<ff_cell_t *> &vec_tmp, ff_cell_t *cell) {
    if (cell->is_floor && !cell->flooded) {
        vec.push_back(cell);
        vec_tmp.push_back(cell);
    }
}