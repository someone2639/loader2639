#include <ultra64.h>
#include "filesystem/ff.h"
#include "fs_api.h"

direntry_t fsFileList[MAX_FILES];
hide_sysfolder = 1;
int page = 0;
int cursor = 0;
int fsFileCount = 0;

void print_dir(direntry_t *ls, int cur, int p, int len, int count) {
    char *toPrint = allocator_malloc(MAX_FILENAME_LEN * count);
    for (int i = 0; i < count; i++) {
        toPrint += sprintf(toPrint, "%c %s\n", cur == count ? '>' : ' ', ls[i].filename);
    }
}

void fs_filelist_compare(direntry_t *a, direntry_t *b) {
    return strcmp(a->filename, b->filename);
}

void fs_ls(char *directory) {
    fsFileCount = 1;
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


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
        *(vs8*)0=0;
    }

    page = 0;
    cursor = 0;

    if (fsFileCount > 0)
    {
        /* Should sort! */
        qsort(fsFileList, fsFileCount, sizeof(direntry_t), fs_filelist_compare);
    }
}


