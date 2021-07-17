#pragma once

#define MAX_LIST 20
#define MAX_FILENAME_LEN 256
#define MAX_FILES 500

enum {
    DT_UNKNOWN = 0,
    DT_FIFO = 1,
    DT_CHR = 2,
    DT_DIR = 4,
    DT_BLK = 6,
    DT_REG = 8,
    DT_LNK = 10,
    DT_SOCK = 12,
    DT_WHT = 14
};

typedef struct {
    u32 type;
    u32 color;
    u8 filename[MAX_FILENAME_LEN + 1];
} direntry_t;
