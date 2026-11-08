#include "arena.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct arena_block {
    struct arena_block *next;
    size_t offset;
    _Alignas(max_align_t) unsigned char data[];
} arena_block;

struct arena {
    arena_block *head;
};

static arena_block *arena_block_create(void) {
    arena_block *ret = malloc(sizeof *ret + BLOCK_SIZE);
    if (ret == NULL) {
        return NULL;
    }
    ret->offset = 0;
    ret->next = NULL;
    return ret;
}

static int arena_block_mem_ok(arena_block *ab, size_t size) {
    size_t aligned = (size + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1);

    return ab->offset <= BLOCK_SIZE && aligned <= BLOCK_SIZE - ab->offset;
}

static void *arena_block_reserve(arena_block *ab, size_t size) {
    void *ret = ab->data + ab->offset;
    ab->offset += (size + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1);

    return ret;
}

void *arena_alloc(arena *a, size_t len, size_t size) {
    if (len == 0 || size == 0 || a == NULL || SIZE_MAX / size < len) {
        return NULL;
    }

    size_t bytes = len * size;

    if (bytes > BLOCK_SIZE) {
        return NULL;
    }

    for (arena_block *curr = a->head; curr != NULL; curr = curr->next) {
        if (arena_block_mem_ok(curr, bytes)) {
            return arena_block_reserve(curr, bytes);
        }
        if (curr->next == NULL) {
            curr->next = arena_block_create();
            if (curr->next == NULL) {
                return NULL;
            }
            return arena_block_reserve(curr->next, bytes);
        }
    }
    return NULL;
}

arena *arena_create(void) {
    arena *ret = malloc(sizeof *ret);
    if (ret == NULL) {
        return NULL;
    }

    ret->head = arena_block_create();
    if (ret->head == NULL) {
        free(ret);
        return NULL;
    }

    return ret;
}

void arena_reset(arena *a) {
    if (a == NULL) {
        return;
    }

    for (arena_block *curr = a->head; curr != NULL; curr = curr->next)
        curr->offset = 0;
}

void arena_destruct(arena *a) {
    if (a == NULL)
        return;

    arena_block *curr = a->head;

    while (curr != NULL) {
        arena_block *next = curr->next;
        free(curr);
        curr = next;
    }

    free(a);
}
