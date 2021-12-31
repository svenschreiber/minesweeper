#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiB(n) ((n) << 10)
#define MiB(n) ((n) << 20)
#define GiB(n) ((u64)(n) << 30)

#define Assert(expression) if(!(expression)) { *(int *)0 = 0; }

#define RGB_NORM(r,g,b) ((f32)r/255.0f), ((f32)g/255.0f), ((f32)b/255.0f)

struct String {
    char *str;
    s32 size;
};

struct Vec2f {
    f32 x;
    f32 y;
};

struct Vec2i {
    s32 x;
    s32 y;
};

#endif
