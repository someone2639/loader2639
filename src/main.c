#include <ultra64.h>
#include <PR/gs2dex.h>


#include "filesystem/ff.h"

extern OSMesgQueue piMessageQ;
extern OSMesgQueue siMessageQ;
extern OSMesgQueue dmaMessageQ;
extern OSMesgQueue rspMessageQ;
extern OSMesgQueue rdpMessageQ;
extern OSMesgQueue retraceMessageQ;

#define WIDTH 320
#define HEIGHT 240

extern u16 gFrameBuffers[2][320][240];

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
    gsDPSetColorImage(G_IM_FMT_RGBA, G_IM_SIZ_16b, 320, 0x10000000),
    gsDPSetCycleType(G_CYC_FILL),
    gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
    gsDPSetScissor(G_SC_NON_INTERLACE, 0, 0, WIDTH, HEIGHT),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
    gsDPSetFillColor(GPACK_RGBA5551(64, 64, 255, 1) << 16 | GPACK_RGBA5551(64, 64, 255, 1)),
    gsDPFillRectangle(0, 0, WIDTH - 1, HEIGHT - 1),
    gsSPEndDisplayList(),
};

#define RDPFIFO_SIZE    (8*1024/sizeof(u64))
#define GLIST_LEN       2048
#define SP_BOOT_UCODE_SIZE      0x00d0
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

void main2(void *arg) {
    u8 draw_frame = 0;
    Gfx *gp;

    while (1) {
        osRecvMesg(&retraceMessageQ, NULL, OS_MESG_BLOCK);

        gp = glist;

        gSPSegment(gp++, 0x10, gFrameBuffers[draw_frame]);
        gSPDisplayList(gp++, clearCfb);

        gDPFullSync(gp++);
        gSPEndDisplayList(gp++);

        tlist.t.data_ptr = (u64 *) glist;
        osWritebackDCache(glist, ((u32) gp) - ((u32) glist));
        osSpTaskStart(&tlist);

        osRecvMesg(&rspMessageQ, NULL, OS_MESG_BLOCK);
        osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);

        osViSwapBuffer(gFrameBuffers[draw_frame]);
        draw_frame ^= 1;
    }
}
