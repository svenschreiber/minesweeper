#ifndef MINESWEEPER_H

#define GAP_MAX_SIZE KiB(2)

struct Game_Board_Cell {
    b32 mine;
    b32 revealed;
    b32 flagged;
    u32 mines_nearby;
};

struct Game_Board {
    Game_Board_Cell grid[24][20];
    s32 size_x;
    s32 size_y;
    u32 time_elapsed;
    u32 flags_available;
};

struct Game_Data {
    Mem_Arena arena_;
    Mem_Arena *arena;
    Game_Board board;
};

#define MINESWEEPER_H
#endif
