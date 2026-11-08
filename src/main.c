#include <stdio.h>
#include "arena/arena.h"

int main(void) {
    arena *a = arena_create();
    int *x = arena_alloc(a, 1, sizeof(int));
    int *y = arena_alloc(a, 1, sizeof(int));

    printf("variable x position %p \n", (void *)x);
    printf("variable y position %p \n", (void *)y);

    return 0;
}
