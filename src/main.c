#include <ultra64.h>
#include <PR/gs2dex.h>

#include "n64_defs.h"
#include "filesystem/ff.h"
#include "s2d_engine/s2d_print.h"

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
u64 system_rspyield[OS_YIELD_DATA_SIZE/sizeof(u64)];

OSTask tlist = {
    M_GFXTASK,                             /* task type                */
    OS_TASK_DP_WAIT | OS_TASK_LOADABLE,    /* task flags               */
    (u64 *) &rspbootTextStart,             /* boot ucode ptr           */
    SP_BOOT_UCODE_SIZE,                    /* boot ucode size          */
    (u64 *) &gspS2DEX2_fifoTextStart,       /* ucode ptr                */
    SP_UCODE_SIZE,                         /* ucode size               */
    (u64 *) &gspS2DEX2_fifoDataStart,       /* ucode data ptr           */
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

#include "s2d_engine/config.h"
#include FONT_C_FILE

extern float sinf(float);

char *get_rom_name(void) {
    u8 *ret = allocator_malloc_dl(32);
    u32 *yeah = ret;

    yeah[0] = *(u32 *)0xB0000020;
    yeah[1] = *(u32 *)0xB0000024;
    yeah[2] = *(u32 *)0xB0000028;
    yeah[3] = *(u32 *)0xB000002C;
    yeah[4] = *(u32 *)0xB0000030;
    ret[20] = 0;

    return ret;
}

void main2(void *arg) {
    u8 draw_frame = 0;

    while (1) {
        gdl_head = glist;

        gSPSegment(gdl_head++, 2, gFrameBuffers[draw_frame]);
        gSPDisplayList(gdl_head++, clearCfb);

        if (gTimer > 0) {
            s2d_init();
            s2d_rdp_init();
            static char d[0x50];
            sprintf(d, SCALE "25" "The ROM name is %s", get_rom_name());
            s2d_print_alloc(50 + (4.0f * sinf(gTimer / 5.0f)), 50, ALIGN_LEFT, d);
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
