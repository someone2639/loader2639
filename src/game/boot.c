#include <ultra64.h>
#include "n64_defs.h"

u64 progStack[STACKSIZE / sizeof(u64)];

extern void main(void *);

static OSThread idleThread;
static u64 idleThreadStack[STACKSIZE / sizeof(u64)];
static OSThread mainThread;
static u64 mainThreadStack[STACKSIZE / sizeof(u64)];

OSMesgQueue piMessageQ;
OSMesgQueue siMessageQ;
OSMesgQueue dmaMessageQ;
OSMesgQueue rspMessageQ;
OSMesgQueue rdpMessageQ;
OSMesgQueue retraceMessageQ;

static OSMesg piMessages[NUM_PI_MSGS];
static OSMesg siMessageBuf;
static OSMesg dmaMessageBuf;
static OSMesg rspMessageBuf;
static OSMesg rdpMessageBuf;
static OSMesg retraceMessageBuf;

OSPiHandle *carthandle;
OSIoMesg dmaIOMessageBuf;

OSContStatus contStatus[MAXCONTROLLERS];
u8 contExist;

void mainproc(void *arg) {
    /* Create Message Queue */
    osCreateMesgQueue(&dmaMessageQ, &dmaMessageBuf, 1);
    osCreateMesgQueue(&rspMessageQ, &rspMessageBuf, 1);
    osCreateMesgQueue(&rdpMessageQ, &rdpMessageBuf, 1);
    osCreateMesgQueue(&siMessageQ, &siMessageBuf, 1);
    osCreateMesgQueue(&retraceMessageQ, &retraceMessageBuf, 1);

    /* Connect Event to Message Queue */
    osSetEventMesg(OS_EVENT_SP, &rspMessageQ, NULL);
    osSetEventMesg(OS_EVENT_DP, &rdpMessageQ, NULL);
    osSetEventMesg(OS_EVENT_SI, &siMessageQ, NULL);
    osViSetEvent(&retraceMessageQ, NULL, 1);

    /* Initialize Controller */
    osContInit(&siMessageQ, &contExist, contStatus);
    carthandle = osCartRomInit();

    crash_screen_init();

    /* Call Main Function */
    main(arg);
}

static void idle(void *arg) {
    /* Activate Vi Manager */
    osCreateViManager(OS_PRIORITY_VIMGR);
    osViSetMode(&osViModeTable[OS_VI_NTSC_LAN1]);

    /* Activate Pi Manager */
    osCreatePiManager((OSPri) OS_PRIORITY_PIMGR, &piMessageQ, piMessages, NUM_PI_MSGS);

    /* Activate Main Thread */
    osCreateThread(&mainThread, 3, mainproc, arg, mainThreadStack + STACKSIZE / sizeof(u64), 10);
    osStartThread(&mainThread);
    osSetThreadPri(0, 0);

    /* Start Idle Loop */
    while (1)
        ;
}

void boot(void) {
    osInitialize();
    osCreateThread(&idleThread, 1, idle, (void *) 0, idleThreadStack + STACKSIZE / sizeof(u64), 10);
    osStartThread(&idleThread);
}

