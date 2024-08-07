#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "arena.h"

// TODO: Implement arena free

Arena arena_init(size_t n_bytes) {
    void *ptr = malloc(n_bytes);

    if (ptr == NULL) {
        printf("Arena init failed.\n");
        exit(1);
    }

    Arena a = {0};

    a.ptr = ptr;
    a.allocated_bytes = 0;
    a.max_size = n_bytes;

    return a;
}

void *arena_alloc(Arena *a, size_t n_bytes) {
    if (n_bytes > a->max_size) {
        printf("Requested bytes: %ld is greater than arena size %ld\n", n_bytes, a->max_size);
        return NULL;
    }

    if (n_bytes + a->allocated_bytes > a->max_size) {
        printf("Requested bytes: %ld will overflow the arena with size %ld which is already allocated with %ld bytes\n", n_bytes, a->max_size,
               a->allocated_bytes);
        return NULL;
    }

    a->allocated_bytes += n_bytes;

    return (char *)a->ptr + n_bytes;
}
