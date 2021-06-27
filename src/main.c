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

extern u16 gFrameBuffers[2][320][240];


OSMesgQueue piMessageQ;
OSMesgQueue siMessageQ;
OSMesgQueue dmaMessageQ;
OSMesgQueue rspMessageQ;
OSMesgQueue rdpMessageQ;
OSMesgQueue retraceMessageQ;

static OSMesg   piMessages[8];
static OSMesg   siMessageBuf;
static OSMesg   dmaMessageBuf;
static OSMesg   rspMessageBuf;
static OSMesg   rdpMessageBuf;
static OSMesg   retraceMessageBuf;

OSIoMesg    dmaIOMessageBuf;

OSContStatus    contStatus[MAXCONTROLLERS];
OSContPad   contPad[MAXCONTROLLERS];
u8      contExist;

void main(void) {
    osInitialize();

    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);

    osCreateMesgQueue(&dmaMessageQ,     &dmaMessageBuf,       1);
    osCreateMesgQueue(&rspMessageQ,     &rspMessageBuf,       1);
    osCreateMesgQueue(&rdpMessageQ,     &rdpMessageBuf,       1);
    osCreateMesgQueue(&siMessageQ,      &siMessageBuf,        1);
    osCreateMesgQueue(&retraceMessageQ, &retraceMessageBuf,   1);

    /* Connect Event to Message Queue */
    osSetEventMesg(OS_EVENT_SP, &rspMessageQ, NULL);
    osSetEventMesg(OS_EVENT_DP, &rdpMessageQ, NULL);
    osSetEventMesg(OS_EVENT_SI, &siMessageQ,  NULL);
    osViSetEvent(&retraceMessageQ, NULL, 1);

    u8 draw_frame = 0;

    Gfx *gdl_head;

    while (1) {
        gdl_head = dl_buffer;
        gDPSetColorImage(gdl_head++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 320,
                     gFrameBuffers[draw_frame]);

        gFrameBuffers[0][0][0] = 0xFFFF;
        gFrameBuffers[0][1][1] = 0xFFFF;
        gFrameBuffers[0][2][2] = 0xFFFF;
        gFrameBuffers[0][3][3] = 0xFFFF;

        gDPFullSync(gdl_head ++);
        gSPEndDisplayList(gdl_head ++);


        tlist.t.data_ptr = (u64 *)dl_buffer;

        osWritebackDCache(dl_buffer, ((u32)gdl_head)-((u32)dl_buffer));
        osSpTaskStart(&tlist);

        /*------ ½ªÎ»ÂÔ¤Á ------*/
        osRecvMesg(&rspMessageQ, NULL, OS_MESG_BLOCK);
        osRecvMesg(&rdpMessageQ, NULL, OS_MESG_BLOCK);   

        draw_frame ^= 1;
    }


}
