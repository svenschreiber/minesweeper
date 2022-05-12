#include "base.h"
#include "platform.h"
#include "memory.h"
#include "opengl.h"
#include "minesweeper.h"

#include "platform.cpp"
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

static Game_Data *game_data = 0;

static u32 flag_tex_id = 0;
static u32 mine_tex_id = 0;

static u32 one_tex_id = 0;
static u32 two_tex_id = 0;
static u32 three_tex_id = 0;
static u32 four_tex_id = 0;
static u32 five_tex_id = 0;

static u32 you_lost_tex_id = 0;
static u32 you_won_tex_id = 0;

#define NUM_MINES 99

#define INFO_BAR_HEIGHT 60
#define INFO_BAR_WIDTH PLATFORM_DEFAULT_WINDOW_WIDTH
#define CELL_SIZE 25

#define INFO_BAR_COLOR RGB_NORM(76, 76, 110)
#define CELL_COLOR_UNREVEALED1 RGB_NORM(117, 129, 172)
#define CELL_COLOR_UNREVEALED2 RGB_NORM(124, 135, 178)
#define CELL_COLOR_REVEALED1 RGB_NORM(171, 177, 193)
#define CELL_COLOR_REVEALED2 RGB_NORM(175, 181, 197)
#define CELL_COLOR_HOVER RGB_NORM(139, 152, 197)

static s32
get_random(s32 lower, s32 upper) {
    s32 num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

static Vec2i
get_board_coord(s32 index) {
    return {index % game_data->board.width, index / game_data->board.height};
}

static void
clear_board() {
    for (int x = 0; x < game_data->board.width; ++x) {
        for (int y = 0; y < game_data->board.height; ++y) {
            Game_Board_Cell *cell = &game_data->board.grid[x][y];
            cell->adj_mines = 0;
            cell->mine = false;
            cell->flagged = false;
            cell->revealed = false;
        }
    }
    game_data->started = false;
    game_data->lost = false;
    game_data->won = false;
}

static b32
check_for_win(Game_Board *board) {
    s32 num_hidden = 0;
    for (s32 y = 0; y < board->height; ++y) {
        for (s32 x = 0; x < board->width; ++x) {
            Game_Board_Cell *cell = &board->grid[x][y];
            if (!cell->revealed) {
                ++num_hidden;
            }
        }
    }

    if (num_hidden == NUM_MINES) {
        return true;
    }

    return false;
}

static void
game_lose(Game_Board *board) {
    game_data->lost = true;
    
    for (s32 y = 0; y < board->height; ++y) {
        for (s32 x = 0; x < board->width; ++x) {
            Game_Board_Cell *cell = &board->grid[x][y];
            if (cell->mine) {
                cell->revealed = true;
                cell->flagged = false;
            }
        }
    }
}

static void
open_cell(Game_Board *board, s32 x, s32 y) {
    if (game_data->lost || game_data->won) return;
    if (x < 0 || x >= board->width || y < 0 || y >= board->height) return; // should never happen
    if (board->grid[x][y].mine) {
        game_lose(board);
        return;
    }

    s32 mine_count = 0;

    s32 adj_cells_x[8];
    s32 adj_cells_y[8];

    s32 adj_cells_count = 0;
    
    for (s32 i = -1; i <= 1; ++i) {
        if (x + i < 0 || x + i >= board->width) continue;
        for (s32 j = -1; j <= 1; ++j) {
            if (y + j < 0 || y + j >= board->height) continue;
            if (i == 0 && j == 0) continue;
            
            adj_cells_x[adj_cells_count] = x + i;
            adj_cells_y[adj_cells_count] = y + j;
            ++adj_cells_count;
            
            if (board->grid[x + i][y + j].mine) {
                ++mine_count;
            }
        }
    }
    board->grid[x][y].revealed = true;
    board->grid[x][y].flagged = false;
    board->grid[x][y].adj_mines = mine_count;

    if (mine_count == 0) {
        for (s32 i = 0; i < adj_cells_count; ++i) {
            s32 adj_x = adj_cells_x[i];
            s32 adj_y = adj_cells_y[i];
            if (!board->grid[adj_x][adj_y].revealed) {
                open_cell(board, adj_x, adj_y);
            }
        }
    }
}

static void
generate_board(Vec2i start) {
    Game_Board *board = &game_data->board;

    s32 num_mines = 0;

    while (num_mines < NUM_MINES) {
        s32 random = get_random(0, board->width * board->height - 1);

        Vec2i mine = get_board_coord(random);
        
        if (abs(start.x - mine.x) > 2 || abs(start.y - mine.y) > 2) {
            if (!board->grid[mine.x][mine.y].mine) {
                board->grid[mine.x][mine.y].mine = true;
                ++num_mines;
            }
        }
    }
    
    open_cell(board, start.x, start.y);
    
}

static Vec2i
get_cell_at_mouse_pos() {
    s32 x = platform_state->mouse_pos.x / CELL_SIZE;
    s32 y = (platform_state->mouse_pos.y - INFO_BAR_HEIGHT)  / CELL_SIZE;
    return {x, y};
}

static b32
is_mouse_in_board() {
    return platform_state->mouse_pos.y >= INFO_BAR_HEIGHT;
}

static u32
load_texture(const char *file_path) {
    u32 tex_id;
    glGenTextures(1, &tex_id);

    s32 width, height, num_channels;
    u8 *data = stbi_load(file_path, &width, &height, &num_channels, STBI_rgb_alpha);
    if (data) {
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        platform_log("Texture failed to load at path: %s", file_path);
    }
    
    stbi_image_free(data);
    return tex_id;
}

static void
game_draw_board() {
    Game_Board *board = &game_data->board;

    // draw info bar
    glBegin(GL_QUADS);
    glColor3f(INFO_BAR_COLOR);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f((f32)INFO_BAR_WIDTH, 0.0f, 0.0f);
    glVertex3f((f32)INFO_BAR_WIDTH, INFO_BAR_HEIGHT, 0.0f);
    glVertex3f(0.0f,  INFO_BAR_HEIGHT, 0.0f);
    glEnd();

    f32 status_size = 0.2f;
    s32 status_width = (s32)(747 * status_size);
    s32 status_height = (s32)(257 * status_size);
    
    s32 status_x = INFO_BAR_WIDTH / 2 - status_width / 2;
    s32 status_y = 7;
    
    Vec2i status_tl = {status_x, status_y};
    Vec2i status_tr = {status_x + status_width, status_y};
    Vec2i status_bl = {status_x, status_y + status_height};
    Vec2i status_br = {status_x + status_width, status_y + status_height};

    if (game_data->lost || game_data->won) {
        glEnable(GL_TEXTURE_2D);

        if (game_data->lost)
            glBindTexture(GL_TEXTURE_2D, you_lost_tex_id);
        if (game_data->won)
            glBindTexture(GL_TEXTURE_2D, you_won_tex_id);
        
        glColor3f(1.0f, 1.0f, 1.0f);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f((f32)status_tl.x, (f32)status_tl.y, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f((f32)status_bl.x, (f32)status_bl.y, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f((f32)status_br.x, (f32)status_br.y, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f((f32)status_tr.x, (f32)status_tr.y, 0.0f);
        glEnd();

        
        glDisable(GL_TEXTURE_2D);
    }

    // draw board
    for (s32 y = 0; y < board->height; ++y) {
        for (s32 x = 0; x < board->width; ++x) {
            
            Vec2i tl = { x * CELL_SIZE, y * CELL_SIZE + INFO_BAR_HEIGHT };
            Vec2i tr = { (x + 1) * CELL_SIZE, tl.y };
            Vec2i bl = { tl.x, (y + 1) * CELL_SIZE + INFO_BAR_HEIGHT };
            Vec2i br = { tr.x, bl.y };

            b32 x_even = !(x % 2);
            b32 y_even = !(y % 2);

            if (y_even && x_even || !(y_even || x_even)) {
                if (board->grid[x][y].revealed) {
                    glColor3f(CELL_COLOR_REVEALED1);
                } else {
                    glColor3f(CELL_COLOR_UNREVEALED1);
                }
            } else {
                if (board->grid[x][y].revealed) {
                    glColor3f(CELL_COLOR_REVEALED2);
                } else {
                    glColor3f(CELL_COLOR_UNREVEALED2);
                }
            }

            // draw cell
            glBegin(GL_QUADS);
            glVertex3f((f32)tl.x, (f32)tl.y, 0.0f);
            glVertex3f((f32)tr.x, (f32)tr.y, 0.0f);
            glVertex3f((f32)br.x, (f32)br.y, 0.0f);
            glVertex3f((f32)bl.x, (f32)bl.y, 0.0f);
            glEnd();

            // draw flag 
            if (board->grid[x][y].flagged) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, flag_tex_id);
                glColor3f(1.0f, 1.0f, 1.0f);

                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f((f32)tl.x, (f32)tl.y, 0.0f);
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f((f32)bl.x, (f32)bl.y, 0.0f);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f((f32)br.x, (f32)br.y, 0.0f);
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f((f32)tr.x, (f32)tr.y, 0.0f);
                glEnd();
                
                glDisable(GL_TEXTURE_2D);
            }

            // draw mine
			if (board->grid[x][y].mine && board->grid[x][y].revealed) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, mine_tex_id);
                
                glColor3f(1.0f, 1.0f, 1.0f);

                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f((f32)tl.x, (f32)tl.y, 0.0f);
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f((f32)bl.x, (f32)bl.y, 0.0f);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f((f32)br.x, (f32)br.y, 0.0f);
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f((f32)tr.x, (f32)tr.y, 0.0f);
                glEnd();
                
                glDisable(GL_TEXTURE_2D);
            }
			
            // draw number
            if (board->grid[x][y].revealed && board->grid[x][y].adj_mines > 0) {
                s32 adj_mines = board->grid[x][y].adj_mines;
                
                glEnable(GL_TEXTURE_2D);

                switch (adj_mines) {
                    case 1: glBindTexture(GL_TEXTURE_2D, one_tex_id); break;
                    case 2: glBindTexture(GL_TEXTURE_2D, two_tex_id); break;
                    case 3: glBindTexture(GL_TEXTURE_2D, three_tex_id); break;
                    case 4: glBindTexture(GL_TEXTURE_2D, four_tex_id); break;
                    case 5: glBindTexture(GL_TEXTURE_2D, five_tex_id); break;
                }
                
                glColor3f(1.0f, 1.0f, 1.0f);

                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f((f32)tl.x, (f32)tl.y, 0.0f);
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f((f32)bl.x, (f32)bl.y, 0.0f);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f((f32)br.x, (f32)br.y, 0.0f);
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f((f32)tr.x, (f32)tr.y, 0.0f);
                glEnd();
                
                glDisable(GL_TEXTURE_2D);
            }

            // draw hover effect
            if (is_mouse_in_board()) {
                Vec2i mouse_cell = get_cell_at_mouse_pos();
                if (mouse_cell.x == x && mouse_cell.y == y) {
                    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
                    
                    glBegin(GL_QUADS);
                    glVertex3f((f32)tl.x, (f32)tl.y, 0.0f);
                    glVertex3f((f32)tr.x, (f32)tr.y, 0.0f);
                    glVertex3f((f32)br.x, (f32)br.y, 0.0f);
                    glVertex3f((f32)bl.x, (f32)bl.y, 0.0f);
                    glEnd();
                }
            }
        }
    }    
}

static void
game_process_events() {
    
    for (int i = 0; i < platform_state->event_count; ++i) {
        Platform_Event *event = &platform_state->events[i];
        switch (event->type) {

            case PLATFORM_EVENT_TYPE_KEY_PRESS: {
                platform_log("%s pressed!\n", get_key_name(event->key_index).str);
                if (event->key_index == KEY_ESCAPE) {
                    platform_state->running = false;
                }
            } break;

            case PLATFORM_EVENT_TYPE_KEY_RELEASE: {
                platform_log("%s released!\n", get_key_name(event->key_index).str);
                if (event->key_index == KEY_SPACE) {
                    clear_board();
                }
            } break;

            case PLATFORM_EVENT_TYPE_CHARACTER_INPUT: {
                platform_log("%c typed!\n", (char)event->character);
            } break;

            case PLATFORM_EVENT_TYPE_MOUSE_PRESS: {
                platform_log("%s pressed!\n", get_key_name(event->key_index).str);
                if (event->key_index == KEY_MOUSE_BUTTON_RIGHT) {
                    if (is_mouse_in_board()) {
                        Game_Board *board = &game_data->board;
                        Vec2i cell_pos = get_cell_at_mouse_pos();
                        Game_Board_Cell *cell = &board->grid[cell_pos.x][cell_pos.y];
                        if (!cell->revealed && !game_data->won && !game_data->lost) {
                            cell->flagged = !cell->flagged;
                        }
                    }
                } else if (event->key_index == KEY_MOUSE_BUTTON_LEFT) {
                    if (is_mouse_in_board()) {
                        Game_Board *board = &game_data->board;
                        Vec2i cell_pos = get_cell_at_mouse_pos();
                        Game_Board_Cell *cell = &board->grid[cell_pos.x][cell_pos.y];
                        if (!cell->revealed && !cell->flagged) {
                            if (!game_data->started) {
                                game_data->started = true;
                                generate_board(cell_pos);
                            }
                            open_cell(board, cell_pos.x, cell_pos.y);
                            if (check_for_win(board)) {
                                game_data->won = true;
                            }
                        }
                    }
                }
            } break;

            case PLATFORM_EVENT_TYPE_MOUSE_RELEASE: {
                platform_log("%s released!\n", get_key_name(event->key_index).str);
            } break;

            case PLATFORM_EVENT_TYPE_MOUSE_MOVE: {
                platform_log("mouse position: %u | %u\n", platform_state->mouse_pos.x, platform_state->mouse_pos.y);
                if (is_mouse_in_board()) {
                    Vec2i cell_at_mouse = get_cell_at_mouse_pos();
                    platform_log("cell position: %u | %u\n", cell_at_mouse.x, cell_at_mouse.y);
                }
            } break;
        }
    }
    platform_state->event_count = 0;
}

void game_init() {
    {
        Mem_Arena arena_ = mem_arena_init(MiB(256));
        game_data = PushData(&arena_, Game_Data, 1);
        memmove(&game_data->arena_, &arena_, sizeof(game_data->arena_));
    }
    Mem_Arena *arena = game_data->arena = &game_data->arena_;

    game_data->board.width = 24;
    game_data->board.height = 20;
    game_data->started = false;
    game_data->lost = false;
    game_data->won = false;

    srand((u32)time(0));
    
    load_gl_functions();
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, platform_state->window_width, platform_state->window_height);
    glOrtho(0.0f, (f32)platform_state->window_width, (f32)platform_state->window_height, 0.0f, 0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    flag_tex_id = load_texture("res/flag.png");
    mine_tex_id = load_texture("res/mine.png");

    one_tex_id = load_texture("res/one.png");
    two_tex_id = load_texture("res/two.png");
    three_tex_id = load_texture("res/three.png");
    four_tex_id = load_texture("res/four.png");
    five_tex_id = load_texture("res/five.png");
    
    you_lost_tex_id = load_texture("res/you_lost.png");
    you_won_tex_id = load_texture("res/you_won.png");
}

void game_update() {
    game_process_events();

    glClear(GL_COLOR_BUFFER_BIT);

    game_draw_board();
}

////////////////////////////////
// Platform Specific

#if defined(BUILD_WIN32)
#include "win32/win32_minesweeper.cpp"
#else
#error No support for specified platform. Make sure the correct define is set in the build file.
#endif
