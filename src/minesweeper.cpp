#include "base.h"
#include "platform.h"
#include "memory.h"
#include "opengl.h"
#include "minesweeper.h"

#include "platform.cpp"
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

static Game_Data *game_data = 0;

static u32 flag_tex_id = 0;


#define INFO_BAR_HEIGHT 60
#define INFO_BAR_WIDTH PLATFORM_DEFAULT_WINDOW_WIDTH
#define CELL_SIZE 25

#define INFO_BAR_COLOR RGB_NORM(76, 76, 110)
#define CELL_COLOR1 RGB_NORM(117, 129, 172)
#define CELL_COLOR2 RGB_NORM(124, 135, 178)
#define CELL_COLOR_HOVER RGB_NORM(139, 152, 197)

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

    // draw board
    for (s32 y = 0; y < board->size_y; ++y) {
        for (s32 x = 0; x < board->size_x; ++x) {
            
            Vec2i tl = { x * CELL_SIZE, y * CELL_SIZE + INFO_BAR_HEIGHT };
            Vec2i tr = { (x + 1) * CELL_SIZE, tl.y };
            Vec2i bl = { tl.x, (y + 1) * CELL_SIZE + INFO_BAR_HEIGHT };
            Vec2i br = { tr.x, bl.y };

            b32 x_even = !(x % 2);
            b32 y_even = !(y % 2);

            if (y_even && x_even || !(y_even || x_even)) {
                glColor3f(CELL_COLOR1);
            } else {
                glColor3f(CELL_COLOR2);
            }

            if (is_mouse_in_board()) {
                Vec2i mouse_cell = get_cell_at_mouse_pos();
                if (mouse_cell.x == x && mouse_cell.y == y) {
                    glColor3f(CELL_COLOR_HOVER);
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
            } break;

            case PLATFORM_EVENT_TYPE_KEY_RELEASE: {
                platform_log("%s released!\n", get_key_name(event->key_index).str);
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
                        board->grid[cell_pos.x][cell_pos.y].flagged = !board->grid[cell_pos.x][cell_pos.y].flagged;
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

    game_data->board.size_x = 24;
    game_data->board.size_y = 20;
    
    load_gl_functions();
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, platform_state->window_width, platform_state->window_height);
    glOrtho(0.0f, (f32)platform_state->window_width, (f32)platform_state->window_height, 0.0f, 0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    flag_tex_id = load_texture("res/flag.png");
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
