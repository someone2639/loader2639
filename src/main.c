#include <ultra64.h>
#include <PR/gs2dex.h>

#include "n64_defs.h"
#include "filesystem/ff.h"
#include "sd_card/sd.h"
#include "game/fs_api.h"
#include "everdrive/everdrive.h"
#include "s2d_engine/s2d_print.h"
#include "debug.h"

extern OSMesgQueue piMessageQ;
extern OSMesgQueue siMessageQ;
extern OSMesgQueue dmaMessageQ;
extern OSMesgQueue rspMessageQ;
extern OSMesgQueue rdpMessageQ;
extern OSMesgQueue retraceMessageQ;

extern u16 gFrameBuffers[][SCREEN_WIDTH * SCREEN_HEIGHT];

void main(void) {
    // allocator_setup();

    // FIL f;

    // f_open(&f, "My Hacks/smb64.z64", FA_READ);

    // u8 *space = allocator_malloc(0x100000);

    // u32 bread;
    // f_read(&f, space, 0x100000, &bread);
    // while (1);
    main2(NULL);
}

Gfx clearCfb[] = {
    gsDPPipeSync(),
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WIDTH, 2 << 24),
    gsDPSetCycleType(G_CYC_FILL),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetFillColor(GPACK_RGBA5551(64, 64, 255, 1) << 16 | GPACK_RGBA5551(64, 64, 255, 1)),
    gsDPFillRectangle(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1),
    gsSPEndDisplayList(),
};

u64 system_rdpfifo[RDPFIFO_SIZE];
u64 system_rspyield[OS_YIELD_DATA_SIZE / sizeof(u64)];

OSTask tlist = {
    M_GFXTASK,                             /* task type                */
    OS_TASK_DP_WAIT | OS_TASK_LOADABLE,    /* task flags               */
    (u64 *) &rspbootTextStart,             /* boot ucode ptr           */
    SP_BOOT_UCODE_SIZE,                    /* boot ucode size          */
    (u64 *) &gspS2DEX2_fifoTextStart,      /* ucode ptr                */
    SP_UCODE_SIZE,                         /* ucode size               */
    (u64 *) &gspS2DEX2_fifoDataStart,      /* ucode data ptr           */
    SP_UCODE_DATA_SIZE,                    /* ucode data size          */
    NULL,                                  /* dram stack      (ÉÔ»ÈÍÑ) */
    0,                                     /* dram stack size (ÉÔ»ÈÍÑ) */
    (u64 *) system_rdpfifo,                /* fifo buffer top          */
    (u64 *) system_rdpfifo + RDPFIFO_SIZE, /* fifo buffer bottom       */
    NULL,                                  /* data ptr       */
    NULL,                                  /* data_size      */
    (u64 *) system_rspyield,               /* yield data ptr           */
    OS_YIELD_DATA_SIZE,                    /* yield data size          */
};

Gfx glist[GLIST_LEN];

u32 gTimer = 0;
Gfx *gdl_head;

char *get_rom_name_with_dma(void) {
    u8 *ret = allocator_malloc_dl(32);
    u32 *yeah = ret;

    dma_read_s(ret, 0xB0000020, 20);
    ret[20] = 0;

    return ret;
}

char *get_a_file(const char *dir) {
    DIR drc;
    static FILINFO fno;

    FRESULT res;

    char *ret = NULL;

    res = f_opendir(&drc, dir);

    if (res == FR_OK) {
        res = f_readdir(&drc, &fno);
        if (res == FR_OK) {
            ret = fno.fname;
        } else {
            ret = "ERROR cant read dir";
        }
    } else {
        ret = "ERROR cant open dir";
    }
    f_closedir(&dir);
    return ret;
}

FATFS loader_fs;

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *) s1 - *(const unsigned char *) s2;
}

int strneql(const char *s1, const char *s2, int len) {
    return (strcmp(s1, s2) == 0);
}

void init_filesystem(void) {
    evd_ulockRegs();
    sleep(100);

    FRESULT result = f_mount(&loader_fs, "", 1);
    if (result != FR_OK) {

    } else {
        // fat_initialized = 1;
    }
}

void cart_configure(void) {
    u16 msg = 0;
    u8 buff[16];
    u16 firm;

    evd_init();
    // REG_MAX_MSG
    evd_setCfgBit(ED_CFG_SDRAM_ON, 0);
    dma_read_s(buff, ROM_ADDR + 0x20, 16);
    memRomRead32(0x38); // TODO: this should be displayed somewhere...
    evd_setCfgBit(ED_CFG_SDRAM_ON, 1);

    firm = evd_getFirmVersion();

    if (strneql("ED64 SD boot", buff, 12)
        && firm >= 0x0116) // TODO: can this be moved before the firmware is loaded?
    {
        sdSetInterface(DISK_IFACE_SD);
    } else {
        sdSetInterface(DISK_IFACE_SPI);
    }
    memSpiSetDma(0);
}

OSContPad conts[MAXCONTROLLERS];

struct Controller {
    u16 button;
    u16 buttonHeld;
    s8 stickX;
    s8 stickY;
};
struct Controller controllers[MAXCONTROLLERS];

void update_controllers(void) {
    osContStartReadData(&siMessageQ);
    osRecvMesg(&siMessageQ, NULL, OS_MESG_BLOCK);

    osContGetReadData(conts);
    for (int i = 0; i < MAXCONTROLLERS; i++) {
        controllers[i].button = (conts[i].button ^ controllers[i].buttonHeld) & conts[i].button;
        controllers[i].buttonHeld = conts[i].button;
        controllers[i].stickX = conts[i].stick_x;
        controllers[i].stickY = conts[i].stick_y;
        // if (conts[i].button == controllers[i].buttonHeld) {
        //     controllers[i].button = 0;
        // } else {
        //     controllers[i].button = conts[i].button;
        // }
        // if (controllers[i].buttonHeld == 0 && controllers[i].button != 0) {
        //     controllers[i].buttonHeld = controllers[i].button;
        // } else {
        //     controllers[i].buttonHeld = 0;
        // }
    }
}
extern u8 _rebootSegmentStart[], _rebootSegmentRomStart[], _rebootSegmentRomEnd[];
int align1 = 5;


#include "game/fs_api.h"

CurDir cur_directory = {
    "/", // name
    {0}, // slash locations
    0,   // top of slash stack
};

int fs_loaded_latch = 0;
int romLoaded = 0;
int romDMALatch = 0;

int num_ls = 0;

void main2(void *arg) {
    u8 draw_frame = 0;

    cart_configure();
    init_filesystem();
    dma_read_s(_rebootSegmentStart, _rebootSegmentRomStart, (u32) _rebootSegmentRomEnd - (u32) _rebootSegmentRomStart);
    
    while (1) {
        update_controllers();

        gdl_head = glist;

        gSPSegment(gdl_head++, 2, gFrameBuffers[draw_frame]);
        gSPDisplayList(gdl_head++, clearCfb);

        if (gTimer > 0) {
            s2d_init();
            s2d_rdp_init();
            static char d[0x50];

            if (fs_loaded_latch == 0) {
                fs_ls(cur_directory.dirname);
                if (fsFileList[0].filename[0] != '\0') {
                    fs_loaded_latch = 1;
            //         num_ls += 1;
                }
            }

            static char mydir[257];
            sprintf(mydir, SCALE "25" SEPARATOR "%d %s", num_ls, cur_directory.dirname);
            s2d_print_alloc(40 + 5 * sinf(gTimer / 10.0f), 10, ALIGN_LEFT, mydir);
            print_dir(fsFileList, cursor, 0, 0, 10);


            if (controllers[0].button & START_BUTTON) {
                // if (romLoaded) {
                    bootRom(fsFileList[cursor]);
                // }
            }

            if (controllers[0].button & A_BUTTON) {
                if (fsFileList[cursor].type == DT_DIR){
                    curdir_Change(&cur_directory, fsFileList[cursor].filename);
                    // fs_loaded_latch = 0;
                    fs_ls(cur_directory.dirname);
                    num_ls += 1;
                } else if (fsFileList[cursor].type == DT_REG) {
                    curdir_Slash(&cur_directory);

                    char fullpath[FULLPATH_LEN];
                    sprintf(fullpath, "%s%s", cur_directory.dirname, fsFileList[cursor].filename);

                    loadrom(fullpath);
                    curdir_UnSlash(&cur_directory);
                }
            }

            if (controllers[0].button & B_BUTTON) {
                curdir_Unchange(&cur_directory);
                fs_ls(cur_directory.dirname);
            }

            if (controllers[0].button & L_TRIG) {
                assert(0, "test %d", num_ls);
            }

            if (controllers[0].button & U_CBUTTONS) {
                cursor--;
                if (cursor < 0) cursor = 0;
            }
            if (controllers[0].button & D_CBUTTONS) {
                cursor++;
                if (cursor > MAX_FILES) cursor = MAX_FILES;
            }
            if (controllers[0].button & R_CBUTTONS) {
                page++;
            }
            if (controllers[0].button & L_CBUTTONS) {
                page--;
                if (page < 0) page = 0;
            }

            s2d_stop();
        }

        gDPFullSync(gdl_head++);
        gSPEndDisplayList(gdl_head++);

        tlist.t.data_ptr = (u64 *) glist;
        tlist.t.data_size = ((u32) gdl_head) - ((u32) glist);
        osWritebackDCache(glist, ((u32) gdl_head) - ((u32) glist));
        osSpTaskStart(&tlist);

        osRecvMesg(&rspMessageQ, NULL, OS_MESG_BLOCK);
        osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);

        osViSwapBuffer(gFrameBuffers[draw_frame]);
        osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);
        draw_frame ^= 1;
        gTimer++;

        allocator_reset();
    }
}
