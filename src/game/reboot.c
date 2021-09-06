// Provided by Wiseguy
#include <ultra64.h>
#include <PR/os_internal_reg.h>
#include <PR/R4300.h>
#include "everdrive/everdrive.h"
#include "io/utils.h"
#include "rom.h"
#include "cic.h"

extern OSThread *__osActiveQueue;
    extern OSThread *__osRunningThread;
    extern int *__osPiRawReadIo;

// #include "reboot.h"

#define static
#define __inline__

u8 rebootStack[256];
void *keepTheGameFromCrashing = &__osPiRawReadIo; // Why???
void *keepTheGameFromCrashing2 = osPiGetStatus; // Why???

#define WAIT_ON_IOBUSY(stat)                                \
    stat = IO_READ(PI_STATUS_REG);                          \
    while (stat & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY)) \
        stat = IO_READ(PI_STATUS_REG);

/**
 * Disables interrupts
 */
void reboot_disable_interrupts()
{
    register u32 flags;
    __asm__ __volatile__("mfc0 %0, $%1" : "=r" (flags) : "i" (C0_SR));
    flags &= ~SR_IE;
    __asm__ __volatile__("mtc0 %0, $%1" : : "r" (flags), "i" (C0_SR));
}
static __inline__ void reboot_enable_interrupts()
{
    register u32 flags;
    __asm__ __volatile__("mfc0 %0, $%1" : "=r" (flags) : "i" (C0_SR));
    flags |= SR_IE;
    __asm__ __volatile__("mtc0 %0, $%1" : : "r" (flags), "i" (C0_SR));
}

static __inline__ s32 reboot_read_word(u32 devAddr, u32 *data)
{
    register u32 stat;
    WAIT_ON_IOBUSY(stat);
    *data = IO_READ((u32)osRomBase | devAddr);
    return 0;
}

static __inline__ s32 reboot_dma_read(u32 devAddr, void *dramAddr, u32 size)
{
    register u32 stat;
    WAIT_ON_IOBUSY(stat);
    IO_WRITE(PI_DRAM_ADDR_REG, K0_TO_PHYS(dramAddr));
    IO_WRITE(PI_CART_ADDR_REG, K1_TO_PHYS((u32)osRomBase | devAddr));
    IO_WRITE(PI_WR_LEN_REG, size - 1);
    return 0;
}

static __inline__ u32 reboot_get_pi_status()
{
    return IO_READ(PI_STATUS_REG);
}

u8 rebootStack[256];

#define ROM         ((vu32 *)0xB0000000)

int align2 = 43;

s16 gCheats = 0;
s16 cic_chip = CIC_6102;

#define emulator (IO_READ(DPC_PIPEBUSY_REG) == 0)

void reboot_game_internal(u32 devAddr)
{
    u32 entryPoint;
    OSThread *curThread;
    // Disable all interrupts
    

    curThread = __osActiveQueue;
    // Destroy all other threads
    while (curThread && (curThread != curThread->tlnext))
    {
        OSThread *next = curThread->tlnext;
        if (curThread != __osRunningThread)
        {
            osDestroyThread(curThread);
        }
        curThread = next;
    }
    // Get the entry point of the pointed to rom header
    reboot_read_word(devAddr + 8, &entryPoint);

    if (cic_chip == CIC_6105) {
        entryPoint -= 0x10000;
    }

    IO_WRITE(VI_V_INT, 0x3FF);
    IO_WRITE(VI_H_LIMITS, 0);
    IO_WRITE(VI_CUR_LINE, 0);

    IO_WRITE(PI_STATUS_REG, 0x03);

    // Set cart latency registers with values specified in ROM
    u32 lat = ROM[0];
    IO_WRITE(PI_BSD_DOM1_LAT_REG, lat & 0xFF);
    IO_WRITE(PI_BSD_DOM1_PWD_REG, lat >> 8);
    IO_WRITE(PI_BSD_DOM1_PGS_REG, lat >> 16);
    IO_WRITE(PI_BSD_DOM1_RLS_REG, lat >> 20);

    // Imitate initial startup DMA
    reboot_dma_read(devAddr + 0x1000, (void*)entryPoint, 0x100000);

    // if (emulator) {
    //     extern u8 _testromSegmentRomStart[];
    //     reboot_read_word(((u32)_testromSegmentRomStart) + 8, &entryPoint);
    //     reboot_dma_read((u32)_testromSegmentRomStart + 0x1000, (void*)entryPoint, 0x100000);
    // }

    // Wait for DMA to finish
    while (reboot_get_pi_status() & (PI_STATUS_DMA_BUSY | PI_STATUS_ERROR));

    asm __volatile__ (
        ".set noreorder;"

        "lui    $t0, 0x8000;"

        // State required by all CICs
        "move   $s3, $zero;"            // osRomType (0: N64, 1: 64DD)
        "lw     $s4, 0x0300($t0);"      // osTvType (0: PAL, 1: NTSC, 2: MPAL)
        "move   $s5, $zero;"            // osResetType (0: Cold, 1: NMI)
        "lui    $s6, %%hi(cic_ids);"    // osCicId (See cic_ids LUT)
        "addu   $s6, $s6, %0;"
        "lbu    $s6, %%lo(cic_ids)($s6);"
        "lw     $s7, 0x0314($t0);"      // osVersion

        // Copy PIF code to RSP IMEM (State required by CIC-NUS-6105)
        "lui    $a0, 0xA400;"
        "lui    $a1, %%hi(imem_start);"
        "ori    $a2, $zero, 0x0008;"
    "1:"
        "lw     $t0, %%lo(imem_start)($a1);"
        "addiu  $a1, $a1, 4;"
        "sw     $t0, 0x1000($a0);"
        "addiu  $a2, $a2, -1;"
        "bnez   $a2, 1b;"
        "addiu  $a0, $a0, 4;"

        // Copy CIC boot code to RSP DMEM
        "lui    $t3, 0xA400;"
        "ori    $t3, $t3, 0x0040;"      // State required by CIC-NUS-6105
        "move   $a0, $t3;"
        "lui    $a1, 0xB000;"
        "ori    $a2, 0x0FC0;"
    "1:"
        "lw     $t0, 0x0040($a1);"
        "addiu  $a1, $a1, 4;"
        "sw     $t0, 0x0000($a0);"
        "addiu  $a2, $a2, -4;"
        "bnez   $a2, 1b;"
        "addiu  $a0, $a0, 4;"

        // Boot with or without cheats enabled?
        "beqz   %1, 2f;"

        // Patch CIC boot code
        "lui    $a1, %%hi(cic_patch_offsets);"
        "addu   $a1, $a1, %0;"
        "lbu    $a1, %%lo(cic_patch_offsets)($a1);"
        "addu   $a0, $t3, $a1;"
        "lui    $a1, 0x081C;"           // "j 0x80700000"
        "ori    $a2, $zero, 0x06;"
        "bne    %0, $a2, 1f;"
        "lui    $a2, 0x8188;"
        "ori    $a2, $a2, 0x764A;"
        "xor    $a1, $a1, $a2;"         // CIC-NUS-6106 encryption
    "1:"
        "sw     $a1, 0x0700($a0);"      // Patch CIC boot code with jump

        // Patch CIC boot code to disable checksum failure halt
        // Required for CIC-NUS-6105
        "ori    $a2, $zero, 0x05;"
        "beql   %0, $a2, 2f;"
        "sw     $zero, 0x06CC($a0);"

        // Go!
    "2:"
        "b 3f;"
        "nop;"
        // "lui    $sp, 0xA400;"
        // "ori    $ra, $sp, 0x1550;"      // State required by CIC-NUS-6105
        // // "jr     $t3;"
        // "ori    $sp, $sp, 0x1FF0;"      // State required by CIC-NUS-6105


    // Table of all CIC IDs
    "cic_ids:"
        ".byte  0x00;"                  // Unused
        ".byte  0x3F;"                  // NUS-CIC-6101
        ".byte  0x3F;"                  // NUS-CIC-6102
        ".byte  0x78;"                  // NUS-CIC-6103
        ".byte  0xAC;"                  // Unused NUS-CIC-5101 hacked to 4 0xAC seed
        ".byte  0x91;"                  // NUS-CIC-6105
        ".byte  0x85;"                  // NUS-CIC-6106
        ".byte  0xDD;"                  // NUS-CIC-5167

    "cic_patch_offsets:"
        ".byte  0x00;"                  // Unused
        ".byte  0x30;"                  // CIC-NUS-6101
        ".byte  0x2C;"                  // CIC-NUS-6102
        ".byte  0x20;"                  // CIC-NUS-6103
        ".byte  0x30;"                  // Unused NUS-CIC-5101 hacked to 4 same patch offset like 6101
        ".byte  0x8C;"                  // CIC-NUS-6105
        ".byte  0x60;"                  // CIC-NUS-6106
        ".byte  0x30;"                  // NUS-CIC-5167

    // These instructions are copied to RSP IMEM; we don't execute them.
    "imem_start:"
        "lui    $t5, 0xBFC0;"
    "1:"
        "lw     $t0, 0x07FC($t5);"
        "addiu  $t5, $t5, 0x07C0;"
        "andi   $t0, $t0, 0x0080;"
        "bnezl  $t0, 1b;"
        "lui    $t5, 0xBFC0;"
        "lw     $t0, 0x0024($t5);"
        "lui    $t3, 0xB000;"

    "3:"

        :                               // outputs
        : "r" (cic_chip),               // inputs
          "r" (gCheats)
        : "$4", "$5", "$6", "$8",       // clobber
          "$11", "$19", "$20", "$21",
          "$22", "$23", "memory"
    );
    // Jump to the game's entry point
    // reboot_enable_interrupts();

    extern u8 _ram_kernelSegmentRomStart[];
    extern int ramkernel_size;
    reboot_dma_read(_ram_kernelSegmentRomStart, 0x80000000, &ramkernel_size);

    while (reboot_get_pi_status() & (PI_STATUS_DMA_BUSY | PI_STATUS_ERROR));


    // clear_ram_and_boot(entryPoint);
    __asm__ __volatile__("jr %0" : : "r" (entryPoint));
    // simulate_boot(CIC_6102, CIC_6102);
    // __asm__ __volatile__("j 0xA4000040");
}
