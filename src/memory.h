#ifndef MEMORY_H
#define MEMORY_H

#define MEM_ARENA_MAX GiB(2)
#define MEM_ARENA_COMMIT_SIZE KiB(4) // Maybe get the vmem page size from the operating system

struct Mem_Arena {
    u64 max;
    u64 alloc_pos;
    u64 commit_pos;
    void *data;
    u64 align;
};

#define PushData(arena,T,c) ( (T*)(mem_arena_push((arena),sizeof(T)*(c))) )

static Mem_Arena
mem_arena_init_with_align(u64 align, u64 size) {
    Mem_Arena arena = {};
    arena.max        = MEM_ARENA_MAX;
    arena.data       = platform_reserve_memory(arena.max);
    arena.alloc_pos  = 0;
    arena.commit_pos = 0;
    arena.align      = align;
    return arena;
}

static Mem_Arena
mem_arena_init(u64 size = MEM_ARENA_MAX) {
    return mem_arena_init_with_align(8, size);
}

static void *
mem_arena_push(Mem_Arena *arena, u64 size) {
    void *mem = 0;
    if (arena->alloc_pos + size > arena->commit_pos) {
        u64 commit_size = size;
        commit_size += MEM_ARENA_COMMIT_SIZE - 1;
        commit_size -= commit_size % MEM_ARENA_COMMIT_SIZE;
        platform_commit_memory((u8 *)arena->data + arena->commit_pos, commit_size);
        arena->commit_pos += commit_size;
    }
    mem = (u8 *)arena->data + arena->alloc_pos;
    u64 pos = arena->alloc_pos + size;
    arena->alloc_pos = (pos + arena->align - 1) & (~(arena->align - 1));
    return mem;
}

static void
mem_arena_pop(Mem_Arena *arena, u64 size) {
    if (size > arena->alloc_pos) {
        size = arena->alloc_pos;
    }
    arena->alloc_pos -= size;
}

static void
mem_arena_release(Mem_Arena *arena) {
    platform_release_memory(arena->data);
}

#endif
