#ifndef CHOPLIB_ARENA
#define CHOPLIB_ARENA

#include <stddef.h>
#include <stdint.h>


#define BLOCK_SIZE 1024
#define ARENA_ALIGNMENT _Alignof(max_align_t)

_Static_assert(
    BLOCK_SIZE % ARENA_ALIGNMENT == 0,
    "BLOCK_SIZE must be a multiple of ARENA_ALIGNMENT"
);

_Static_assert(
    (ARENA_ALIGNMENT & (ARENA_ALIGNMENT - 1)) == 0,
    "ARENA_ALIGNMENT must be a power of two"
);

_Static_assert(
    BLOCK_SIZE <= SIZE_MAX - (ARENA_ALIGNMENT - 1),
    "Alignment calculation may overflow"
);


typedef struct arena arena;

arena *arena_create(void);

void *arena_alloc(arena *a, size_t len, size_t size);
void arena_reset(arena *a);
void arena_destruct(arena *a);

#endif
