#include "base.h"
#include "platform.h"
#include "memory.h"
#include "opengl.h"
#include "minesweeper.h"

#include "platform.cpp"
#include <windows.h>

static Game_Data *game_data = 0;


#define INFO_BAR_HEIGHT 60
#define INFO_BAR_WIDTH PLATFORM_DEFAULT_WINDOW_WIDTH
#define CELL_SIZE 25

#define INFO_BAR_COLOR RGB_NORM(76, 76, 110)
#define CELL_COLOR1 RGB_NORM(117, 129, 172)
#define CELL_COLOR2 RGB_NORM(124, 135, 178)
#define CELL_COLOR_HOVER RGB_NORM(139, 152, 197)

static b32
is_mouse_in_cell(Vec2i mouse_pos, Vec2i tl, Vec2i br) {
    if (mouse_pos.x >= tl.x && mouse_pos.x < br.x && mouse_pos.y >= tl.y && mouse_pos.y < br.y)
        return true;
    return false;
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

    b32 x_even = true;
    b32 y_even = true;
    b32 color1 = true;

    // draw cells
    for (s32 y = 0; y < board->size_y; ++y) {
        for (s32 x = 0; x < board->size_x; ++x) {

            if (y_even && x_even || !y_even && !x_even) color1 = true;
            else color1 = false;
            
            Vec2i tl = { x * CELL_SIZE, y * CELL_SIZE + INFO_BAR_HEIGHT };
            Vec2i tr = { (x + 1) * CELL_SIZE, tl.y };
            Vec2i bl = { tl.x, (y + 1) * CELL_SIZE + INFO_BAR_HEIGHT };
            Vec2i br = { tr.x, bl.y };

            glBegin(GL_QUADS);

            if (color1)
                glColor3f(CELL_COLOR1);
            else
                glColor3f(CELL_COLOR2);

            if (is_mouse_in_cell(platform_state->mouse_pos, tl, br))
                glColor3f(CELL_COLOR_HOVER);
            
            glVertex3f((f32)tl.x, (f32)tl.y, 0.0f);
            glVertex3f((f32)tr.x, (f32)tr.y, 0.0f);
            glVertex3f((f32)br.x, (f32)br.y, 0.0f);
            glVertex3f((f32)bl.x, (f32)bl.y, 0.0f);
        
            glEnd();
            x_even = !x_even;
        }
        y_even = !y_even;
    }
    
}

static void
game_process_events() {
    // platform_log("event count: %u\n", platform_state->event_count);
    
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
            } break;

            case PLATFORM_EVENT_TYPE_MOUSE_RELEASE: {
                platform_log("%s released!\n", get_key_name(event->key_index).str);
            } break;

            case PLATFORM_EVENT_TYPE_MOUSE_MOVE: {
                platform_log("mouse position: %u | %u\n", platform_state->mouse_pos.x, platform_state->mouse_pos.y);
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
