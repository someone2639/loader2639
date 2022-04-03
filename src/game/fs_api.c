#include <ultra64.h>
#include <PR/gs2dex.h>
#include "types.h"
#include "filesystem/ff.h"
#include "s2d_engine/s2d_print.h"
#include "fs_api.h"
#include "debug.h"

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


    for (int i = cur; i < cur + count; i++) {
        // toPrint += sprintf(toPrint, "%c %s\n", cur == count ? '>' : ' ', ls[i].filename);
        if (i > MAX_FILES) {
            continue;
        }
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

char *fresults[] = {
    "FR_OK",                  /* (0) Succeeded */
    "FR_DISK_ERR",            /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR",             /* (2) Assertion failed */
    "FR_NOT_READY",           /* (3) The physical drive cannot work */
    "FR_NO_FILE",             /* (4) Could not find the file */
    "FR_NO_PATH",             /* (5) Could not find the path */
    "FR_INVALID_NAME",        /* (6) The path name format is invalid */
    "FR_DENIED",              /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST",               /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT",      /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED",     /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE",       /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED",         /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM",       /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED",        /* (14) The f_mkfs() aborted due to any problem */
    "FR_TIMEOUT",             /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED",              /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE",     /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES", /* (18) Number of open files > FF_FS_LOCK */
    "FR_INVALID_PARAMETER"    /* (19) Given parameter is invalid */
};


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
    } else {
        assert (0, "DIR ERROR %s", fresults[res]);
    }

    page = 0;
    cursor = 0;

    // assert(fsFileCount > 0, "No Files here %s", directory);
    // {
    //     /* Should sort! */
    //     qsort(fsFileList, fsFileCount, sizeof(direntry_t), fs_filelist_compare);
    // }
    
}

void curdir_Slash(CurDir *c) {
    c->dirname[c->slashLocations[c->slashIdx]] = '/';
}
void curdir_UnSlash(CurDir *c) {
    c->dirname[c->slashLocations[c->slashIdx]] = ' ';
}


void curdir_Change(CurDir *c, String s) {
    int addlen = strlen(s);
    int origlen = strlen(c->dirname);
    strcpy(&c->dirname[c->slashLocations[c->slashIdx] + 1], s);
    c->slashLocations[++c->slashIdx] = origlen + addlen;
    c->dirname[c->slashLocations[c->slashIdx]] = '/';
    curdir_UnSlash(c);
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


