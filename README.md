
# arena

A small, experimental arena allocator written in C11.

This project is an exercise in manual memory management: allocating fixed-size blocks, handing out aligned slices of memory, and reclaiming all allocations together. It is intentionally small. It is **not** intended to replace `malloc`, and it is not a general-purpose allocator.

**Authorship:** The allocator implementation was written by the repository author. The test code was written with assistance from ChatGPT. Design and implementation decisions were explored iteratively.

## How it works

An arena owns a singly linked list of memory blocks. Each block contains:

- `next`: a pointer to the next block;
- `offset`: the number of bytes already reserved in that block;
- `data[]`: a C99 flexible array member holding the block's usable storage.

A block's storage is allocated together with its metadata using one `malloc` call:

```c
malloc(sizeof(arena_block) + BLOCK_SIZE);
```

The allocator starts with one block. On each `arena_alloc` call it:

1. Checks the requested element count and size for invalid input and multiplication overflow.
2. Computes the requested number of bytes (`len * size`). Requests larger than `BLOCK_SIZE` are rejected.
3. Walks the block list from the head, looking for a block with enough space, including alignment padding.
4. Returns an address inside that block and advances its `offset` when a suitable block is found.
5. Appends a new fixed-size block if no existing block has room, then allocates from it. If `malloc` fails, it returns `NULL`.

Allocation within a block is **bump allocation**: there is no per-object bookkeeping, and individual objects cannot be freed. Blocks are allocated separately rather than resized with `realloc`, so creating a new block does not relocate allocations in older blocks.

```text
arena
  head
   |
   v
+---------------------+     +---------------------+
| block 1             | --> | block 2             | --> NULL
| next / offset       |     | next / offset       |
| data[BLOCK_SIZE]    |     | data[BLOCK_SIZE]    |
+---------------------+     +---------------------+
```

`arena_reset` sets every block's `offset` back to zero, allowing its storage to be reused. It **keeps the allocated blocks** for future use; it does not return them to the system. All pointers obtained before a reset must be treated as invalid for subsequent use. `arena_destruct` traverses the list, frees each block, and then frees the arena itself.

## Alignment and compile-time checks

The block's flexible array is declared with `_Alignas(max_align_t)` so that the compiler places `data[]` at an address with fundamental alignment. The allocator uses `ARENA_ALIGNMENT`, defined using `_Alignof(max_align_t)`, and rounds each reservation up to that boundary.

For a power-of-two alignment, the rounding expression is:

```c
(size + ARENA_ALIGNMENT - 1) & ~(ARENA_ALIGNMENT - 1)
```

The `&` mask clears the low bits after adding `alignment - 1`. This makes the next reservation begin at an aligned offset, provided the base of `data[]` is aligned too. The extra bytes are padding, not additional requested payload.

The `_Static_assert` checks exist because `BLOCK_SIZE` and `ARENA_ALIGNMENT` are compile-time configuration constants. They verify the assumptions behind this bitwise rounding formula **before the program runs**: the alignment must be a power of two, the block size must be a multiple of that alignment, and the rounding addition must not overflow for a valid block-sized request. An ordinary `assert` checks a condition at runtime and can be disabled by `NDEBUG`; `_Static_assert` is appropriate for these fixed configuration invariants.

This allocator guarantees only its configured fundamental alignment, not arbitrary over-aligned types such as objects requiring 32- or 64-byte alignment.

## Example

```c
#include "arena.h"
#include <stdio.h>

int main(void) {
    arena *a = arena_create();
    if (a == NULL) return 1;

    int *x = arena_alloc(a, 1, sizeof *x);
    int *y = arena_alloc(a, 1, sizeof *y);
    if (x == NULL || y == NULL) {
        arena_destruct(a);
        return 1;
    }

    *x = 42;
    *y = 100;
    printf("%d %d\n", *x, *y);

    arena_reset(a); /* x and y must no longer be used */
    arena_destruct(a);
    return 0;
}
```

## Build and tests

Repository layout:

```text
makefile
src/
  arena/
    arena.c
    arena.h
  main.c
  test/
    test.c
```

```sh
make         # build the example
make run     # run the example
make test    # build and run tests with sanitizers
make clean   # remove build output
```

The tests cover basic allocation, block rollover, alignment, reset, invalid requests, and overflow cases. The test suite was written with assistance from ChatGPT; **the arena implementation itself was written by the author**. The initial sanitizer-enabled test run reported `All arena tests passed.` Passing tests is not a proof of correctness, especially for a memory allocator.

## Deliberate limitations

- All blocks have the same compile-time `BLOCK_SIZE`.
- A single request larger than one block returns `NULL`; allocations do not span blocks.
- Each allocation scans the block list from the head. Searching may take **O(number of blocks)**, although reserving space inside an individual block is O(1).
- There is no individual `free`; storage is reused by `arena_reset` or released by `arena_destruct`.
- Reset retains all blocks, including any that are currently unused.
- The allocator is not thread-safe. Concurrent operations on the same arena require external synchronization.
- Object destructors and ownership of resources referenced by stored pointers are not managed.

## Why leave it here?

The goal was to understand and implement the core mechanisms, not to keep adding features until the project becomes a full memory-management library. Fixed-size blocks, a linked list, bump offsets, and fundamental alignment are enough to demonstrate the design and to reuse it in small experiments.

Possible future exercises include variable-sized blocks, a dedicated active-block pointer, allocation marks and rewind, per-type alignment, and benchmarks against ordinary `malloc`/`free`. Those are intentionally **out of scope for this version**.

A tiny allocator that is understandable from top to bottom is the point of this project.
