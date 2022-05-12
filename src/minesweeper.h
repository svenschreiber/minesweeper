#ifndef MINESWEEPER_H

#define GAP_MAX_SIZE KiB(2)

struct Game_Board_Cell {
    b32 mine;
    b32 revealed;
    b32 flagged;
    u32 adj_mines;
};

struct Game_Board {
    Game_Board_Cell grid[24][20];
    s32 width;
    s32 height;
    u32 time_elapsed;
    u32 flags_available;
};

struct Game_Data {
    Mem_Arena arena_;
    Mem_Arena *arena;
    Game_Board board;
    b32 started;
    b32 lost;
    b32 won;
};

#define MINESWEEPER_H
#endif
