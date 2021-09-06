#include <ultra64.h>
#include <PR/gs2dex.h>
#include "types.h"
#include "filesystem/ff.h"
#include "s2d_engine/s2d_print.h"
#include "fs_api.h"

direntry_t fsFileList[MAX_FILES];
hide_sysfolder = 1;
int page = 0;
int cursor = 0;
int fsFileCount = 0;

void print_dir(direntry_t *ls, int cur, int start, int len, int count) {
    // char *toPrint = SCALE "25" "Hello";
    char *toPrint = allocator_malloc(MAX_FILENAME_LEN * count);
    char *head = toPrint;
    toPrint += sprintf(toPrint, SCALE "25");

    // i dont want to error-check that for loop it's already too
    // delicate
    if (start > count) return;


    for (int i = start; i < start + count; i++) {
        // toPrint += sprintf(toPrint, "%c %s\n", cur == count ? '>' : ' ', ls[i].filename);
        toPrint += sprintf(toPrint, "%c %s\n", cur == i ? '>' : ' ', ls[i].filename);
        // toPrint += sprintf(toPrint, "test\n");
    }
    s2d_print_alloc(20, 30, ALIGN_LEFT, head);
}

void fs_filelist_compare(direntry_t *a, direntry_t *b) {
    return strcmp(a->filename, b->filename);
}

void clear_filenames(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        for (int j = 0; j < MAX_FILENAME_LEN; j++) {
            fsFileList[i].filename[j] = 0;
        }
    }
}

void fs_ls(char *directory) {
    fsFileCount = 1;
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    clear_filenames();


    res = f_opendir(&dir, directory);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (!strcmp(fno.fname, "System Volume Information") == 0 || (!strcmp(fno.fname, "ED64") == 0 && hide_sysfolder == 0))
            {
                if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                    fsFileList[fsFileCount - 1].type = DT_DIR;
                } else {                                       /* It is a file. */
                    fsFileList[fsFileCount - 1].type = DT_REG;
                }
                strcpy(fsFileList[fsFileCount - 1].filename, fno.fname);
                fsFileList[fsFileCount - 1].color = 0;
                fsFileCount++;
            }
        }
        f_closedir(&dir);
        fsFileCount--;
    }
    else
    {
        // failed, idk what to do here
        // *(vs8*)0=0;
    }

    page = 0;
    cursor = 0;

    // if (fsFileCount > 0)
    // {
    //     /* Should sort! */
    //     qsort(fsFileList, fsFileCount, sizeof(direntry_t), fs_filelist_compare);
    // }
    
}


void curdir_Change(CurDir *c, String s) {
    int addlen = strlen(s);
    int origlen = strlen(c->dirname);
    strcpy(&c->dirname[c->slashLocations[c->slashIdx] + 1], s);
    c->slashLocations[++c->slashIdx] = origlen + addlen;
    c->dirname[c->slashLocations[c->slashIdx]] = '/';
}

static void strtrim(String dst, int x) {
    for (int i = x; i < FULLPATH_LEN; i++) {
        dst[i] = 0;
    }
}

void curdir_Unchange(CurDir *c) {
    strtrim(c->dirname, c->slashLocations[c->slashIdx - 1] + 1);
    c->slashIdx--;
}


