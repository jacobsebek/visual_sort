#include "SDL.h"

#include <stdbool.h>

#define COL_1 {255, 255, 0}
#define COL_2 {255,200,0}
#define COL_3 {0,255,0}

#define ARRSWAP(arr, a, b) {register int *ap = vals+(a), *bp = vals+(b); register int tmp = *bp; *bp = *ap; *ap = tmp;}
#define arrlen(arr) (sizeof(arr)/sizeof(arr[0]))
#define sign(a) (a == 0 ? 0 : (a > 0 ? 1 : -1))

#define STEP_CONTINUE 0
#define STEP_ABORT 1

typedef unsigned int uint;

typedef struct column_range {
    int l, r;
    SDL_Color color;
} column_range;

bool sort_check();
void sort_merge();
void sort_selection();
void sort_bubble();
void sort_coctail();
void sort_heap();
void sort_quick();
void sort_insertion();
void sort_intro();
void sort_shell();
void sort_radix(); //lsd