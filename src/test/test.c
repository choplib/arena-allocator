
#include "../arena/arena.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


static void test_invalid_requests(void) {
    arena *a = arena_create();
    assert(a != NULL);
    assert(arena_alloc(NULL, 1, 1) == NULL);
    assert(arena_alloc(a, 0, 1) == NULL);
    assert(arena_alloc(a, 1, 0) == NULL);
    assert(arena_alloc(a, SIZE_MAX, 2) == NULL);
    assert(arena_alloc(a, BLOCK_SIZE + 1, 1) == NULL);
    arena_destruct(a);
}

static void test_alignment_and_independence(void) {
    arena *a = arena_create();
    assert(a != NULL);
    unsigned char *one = arena_alloc(a, 1, 1);
    int *number = arena_alloc(a, 1, sizeof *number);
    double *decimal = arena_alloc(a, 1, sizeof *decimal);
    assert(one && number && decimal);
    assert((uintptr_t)one % ARENA_ALIGNMENT == 0);
    assert((uintptr_t)number % ARENA_ALIGNMENT == 0);
    assert((uintptr_t)decimal % ARENA_ALIGNMENT == 0);
    *one = 0x5a;
    *number = 123456;
    *decimal = 3.25;
    assert(*one == 0x5a && *number == 123456 && *decimal == 3.25);
    arena_destruct(a);
}

static void test_multiple_blocks_and_reset(void) {
    arena *a = arena_create();
    assert(a != NULL);
    unsigned char *first = arena_alloc(a, BLOCK_SIZE, 1);
    assert(first != NULL);
    memset(first, 0xa5, BLOCK_SIZE);
    unsigned char *second = arena_alloc(a, BLOCK_SIZE, 1);
    assert(second != NULL && second != first);
    memset(second, 0x5a, BLOCK_SIZE);
    assert(first[0] == 0xa5 && first[BLOCK_SIZE - 1] == 0xa5);
    assert(second[0] == 0x5a && second[BLOCK_SIZE - 1] == 0x5a);
    arena_reset(a);
    // reset invalidates existing allocations; addresses are only compared, never dereferenced afterward.
    unsigned char *again = arena_alloc(a, BLOCK_SIZE, 1);
    assert(again == first);
    arena_destruct(a);
}

static void test_many_small_allocations(void) {
    arena *a = arena_create();
    assert(a != NULL);
    for (size_t i = 0; i < 10000; ++i) {
        uint64_t *p = arena_alloc(a, 1, sizeof *p);
        assert(p != NULL);
        assert((uintptr_t)p % ARENA_ALIGNMENT == 0);
        *p = (uint64_t)i;
        assert(*p == (uint64_t)i);
    }
    arena_destruct(a);
}

int main(void) {
    test_invalid_requests();
    test_alignment_and_independence();
    test_multiple_blocks_and_reset();
    test_many_small_allocations();
    puts("All arena tests passed.");
    return 0;
}
