#ifndef ARENA
#define ARENA

#include <stdio.h>

struct _Arena {
    void *ptr;
    size_t allocated_bytes;
    size_t max_size;
};
typedef struct _Arena Arena;

void *arena_alloc(Arena *a, size_t n_bytes);
Arena arena_init(size_t n_bytes);

#endif // !ARENA
