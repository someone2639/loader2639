#include <ultra64.h>
#include <PR/gs2dex.h>

#define DLBUF_SIZE 6400
#define SP_BOOT_UCODE_SIZE 0xD0

u64 progStack[0x2000];
u64 dl_buffer[DLBUF_SIZE];
u64 dl_yieldbuffer[DLBUF_SIZE];

OSTask tlist = {
    M_GFXTASK,                             /* task type                */
    OS_TASK_DP_WAIT | OS_TASK_LOADABLE,    /* task flags               */
    (u64 *) &rspbootTextStart,             /* boot ucode ptr           */
    SP_BOOT_UCODE_SIZE,                    /* boot ucode size          */
    (u64 *) &gspS2DEX2_fifoTextStart,            /* ucode ptr                */
    SP_UCODE_SIZE,                         /* ucode size               */
    (u64 *) &gspS2DEX2_fifoDataStart,            /* ucode data ptr           */
    SP_UCODE_DATA_SIZE,                    /* ucode data size          */

    NULL,
    0,
    
    (u64 *) dl_buffer,                /* fifo buffer top            */
    (u64 *) dl_buffer + DLBUF_SIZE, /* fifo buffer bottom         */
    NULL, // master DL head
    NULL,
    // yield stuff
    (u64 *) dl_yieldbuffer,
    DLBUF_SIZE,
};


void main(void) {
    osInitialize();

    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);




}
