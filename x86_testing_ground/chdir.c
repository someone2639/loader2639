#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef u8 *String;
#define MAX_DIR_DEPTH 10

typedef struct curdir_t {
    u8 dirname[80 + 1];
    u8 slashLocations[MAX_DIR_DEPTH + 1];
    u8 slashIdx;
} CurDir;


void curdir_Change(CurDir *c, String s) {
    int addlen = strlen(s); // TODO: update slashLocations too IDIOT!
    int origlen = strlen(c->dirname);
    strcpy(&c->dirname[c->slashLocations[c->slashIdx] + 1], s);
    c->slashLocations[++c->slashIdx] = origlen + addlen;
    c->dirname[c->slashLocations[c->slashIdx]] = '/';
}

static void strtrim(String dst, int x) {
    for (int i = x; i < 81; i++) {
        dst[i] = 0;
    }
}

void curdir_Unchange(CurDir *c) {
    strtrim(c->dirname, c->slashLocations[c->slashIdx - 1] + 1);
    c->slashIdx--;
    // c->slashLocations[c->slashIdx + 1] = -1;
}

CurDir mydir = {
    "/",
    {0},
    0,
};

void printdir(CurDir *c) {
    for(int i = 0; i < 80; i++) {
        printf("%c", c->dirname[i] == 0 ? ' ' : c->dirname[i]);
    }
    printf("\n");
    for (int i = 0; i < MAX_DIR_DEPTH; i++) {
        printf("    %d %c\n", c->slashLocations[i], c->dirname[c->slashLocations[i]]);
    }
}


int main(void) {
    printdir(&mydir);
    curdir_Change(&mydir, "mario");
    printdir(&mydir);
    curdir_Change(&mydir, "luig");
    printdir(&mydir);
    curdir_Unchange(&mydir);
    printdir(&mydir);
    curdir_Change(&mydir, "y");
    printdir(&mydir);
    curdir_Change(&mydir, "wario");
    printdir(&mydir);
    curdir_Change(&mydir, "walui");
    printdir(&mydir);
}

