//
// Copyright (c) 2017 The Altra64 project contributors
// Portions (c) 2013 saturnu (Alt64) based on libdragon, Neo64Menu, ED64IO, libn64-hkz, libmikmod
// See LICENSE file in the project root for full license information.
//

#include <ultra64.h>
#include <stdint.h>
#include "sys.h"
#include "rom.h"

#define ALIGN16(val) (((val) + 0xF) & ~0xF)

u32 asm_date;

Options_st options;

u32 native_tv_mode;


#define TICKS_PER_SECOND (93750000/2)

static inline volatile unsigned long get_ticks_ms(void) {
    return TICKS_READ() / (TICKS_PER_SECOND / 1000);
}

extern OSMesgQueue dmaMessageQ;
extern OSPiHandle *carthandle;

// an actual DMA copy
void dma_copy(OSPiHandle *handle, u32 physAddr, u32 vAddr, u32 size, u8 direction) {
    OSIoMesg dmaIoMesg;

    if (direction == OS_WRITE) {
        osWritebackDCache((void*)vAddr, size);
    } else {
        osInvalDCache((void*)vAddr, size);
    }
    dmaIoMesg.hdr.pri = 0;
    dmaIoMesg.hdr.retQueue = &dmaMessageQ;
    dmaIoMesg.size = 0x10000;
    while (size >= 0x10001) {
        dmaIoMesg.dramAddr = (void*)vAddr;
        dmaIoMesg.devAddr = physAddr;
        if (osEPiStartDma(handle, &dmaIoMesg, direction) == -1) {
            *(vs8*)0=0;
        }
        osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
        size -= 0x10000;
        physAddr += 0x10000;
        vAddr += 0x10000;
    }
    if (size != 0) {
        dmaIoMesg.dramAddr = (void*)vAddr;
        dmaIoMesg.devAddr = physAddr;
        dmaIoMesg.size = size;
        if (osEPiStartDma(handle, &dmaIoMesg, direction) == -1) {
            *(vs8*)0=0;
        }
        osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
    }
}

void dma_read_s(void *vAddr, unsigned long physAddr, unsigned long size) {
    dma_copy(carthandle, physAddr, (u32)vAddr, size, OS_READ);
}

void dma_write_s(void *vAddr, unsigned long physAddr, unsigned long size) {
	dma_copy(carthandle, physAddr, (u32)vAddr, size, OS_WRITE);
}

void sleep(u32 ms) {

    u32 current_ms = get_ticks_ms();

    while (get_ticks_ms() - current_ms < ms);

}

void dma_read_sram(void *dest, u32 offset, u32 size) {
    /*
        PI_DMAWait();

        IO_WRITE(PI_STATUS_REG, 0x03);
        IO_WRITE(PI_DRAM_ADDR_REG, K1_TO_PHYS(dest));
        IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset));
       // data_cache_invalidate_all();
        IO_WRITE(PI_WR_LEN_REG, (size - 1));
        */
     /* 0xA8000000
     *  0xb0000000
     *  0x4000000
     * */
    dma_read_s(dest, 0xA8000000 + offset, size);
    //data_cache_invalidate(dest,size);

}

void dma_write_sram(void* src, u32 offset, u32 size) {
    /*
        PI_DMAWait();

        IO_WRITE(PI_STATUS_REG, 0x02);
        IO_WRITE(PI_DRAM_ADDR_REG, K1_TO_PHYS(src));
        IO_WRITE(PI_CART_ADDR_REG, (0xA8000000 + offset));
      //  data_cache_invalidate_all();
        IO_WRITE(PI_RD_LEN_REG, (size - 1));
	*/
    dma_write_s(src, 0xA8000000 + offset, size);

}






u32 ii;
volatile u32 *pt;
void clean();

#define MEM32(addr) *((volatile u32 *)addr)


u8 STR_intToDecString(u32 val, u8 *str) {

    int len;

    if (val < 10)len = 1;
    else
        if (val < 100)len = 2;
    else
        if (val < 1000)len = 3;
    else
        if (val < 10000)len = 4;
    else
        if (val < 100000)len = 5;
    else
        if (val < 1000000)len = 6;
    else
        if (val < 10000000)len = 7;
    else
        if (val < 100000000)len = 8;
    else
        if (val < 1000000000)len = 9;
    else len = 10;

    str += len;
    str[0] = 0;
    if (val == 0)*--str = '0';
    while (val) {

        *--str = '0' + val % 10;
        val /= 10;
    }


    return len;
}

void STR_intToDecStringMin(u32 val, u8 *str, u8 min_size) {

    int len;
    u8 i;

    if (val < 10)len = 1;
    else
        if (val < 100)len = 2;
    else
        if (val < 1000)len = 3;
    else
        if (val < 10000)len = 4;
    else
        if (val < 100000)len = 5;
    else
        if (val < 1000000)len = 6;
    else
        if (val < 10000000)len = 7;
    else
        if (val < 100000000)len = 8;
    else
        if (val < 1000000000)len = 9;
    else len = 10;

    if (len < min_size) {

        i = min_size - len;
        while (i--)str[i] = '0';
        len = min_size;
    }
    str += len;
    str[0] = 0;
    if (val == 0)*--str = '0';
    while (val) {

        *--str = '0' + val % 10;
        val /= 10;
    }
}



