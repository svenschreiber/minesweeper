#ifndef PLATFORM_H
#define PLATFORM_H


// General
#define PLATFORM_DEFAULT_WINDOW_WIDTH  616
#define PLATFORM_DEFAULT_WINDOW_HEIGHT 599


// Events
#define PLATFORM_EVENT_TYPE_NONE            0x00
#define PLATFORM_EVENT_TYPE_KEY_PRESS       0x01
#define PLATFORM_EVENT_TYPE_KEY_RELEASE     0x02
#define PLATFORM_EVENT_TYPE_CHARACTER_INPUT 0x03
#define PLATFORM_EVENT_TYPE_MOUSE_PRESS     0x04
#define PLATFORM_EVENT_TYPE_MOUSE_RELEASE   0x05
#define PLATFORM_EVENT_TYPE_MOUSE_MOVE      0x06
#define PLATFORM_EVENT_TYPE_MOUSE_SCROLL    0x07

struct Platform_Event {
    s32 type;
    s32 key_index;
    s32 key_modifiers;
    u32 character;
};

struct Platform_File {
    u64 size;
    u8 *data;
};

struct Platform_State {
    s32 window_width;
    s32 window_height;
    b32 running;

    Vec2i mouse_pos;
    
    u64 event_count;
    Platform_Event events[2048];
};

static Platform_State *platform_state = 0;
    
struct Mem_Arena;


// Platform functions with platform specific implementation.
void* platform_get_gl_proc_address(char *function_name);
void* platform_reserve_memory(u64 size);
void platform_commit_memory(void *mem, u64 size);
void platform_release_memory(void *mem);
void platform_decommit(void *mem, u64 size);
// NOTE(sven): On windows we can only read files up to 4GiB, if they're bigger, they might not fully load and get truncated at the end. Even if the file size is read correctly.
b32 platform_read_file(Mem_Arena *arena, char *filename, Platform_File *file_result);
void platform_log(char *format, ...);

// Platform functions with general implementation.
static void platform_push_event(Platform_Event event);


#include "key_input.h"

#endif
