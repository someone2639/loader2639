#include <ultra64.h>
#include "filesystem/ff.h"
#include "everdrive/everdrive.h"

enum strtrim_mode_t {
    STRLIB_MODE_ALL       = 0, 
    STRLIB_MODE_RIGHT     = 0x01, 
    STRLIB_MODE_LEFT      = 0x02, 
    STRLIB_MODE_BOTH      = 0x03
};

char *strcpytrim(char *d, // destination
                 char *s, // source
                 int mode,
                 char *delim
                 ) {
    char *o = d; // save orig
    char *e = 0; // end space ptr.
    char dtab[256] = {0};
    if (!s || !d) return 0;

    if (!delim) delim = " \t\n\f";
    while (*delim) 
        dtab[*delim++] = 1;

    while ( (*d = *s++) != 0 ) { 
        if (!dtab[*d]) { // Not a match char
            e = 0;       // Reset end pointer
        } else {
            if (!e) e = d;  // Found first match.

            if ( mode == STRLIB_MODE_ALL || ((mode != STRLIB_MODE_RIGHT) && (d == o)) ) 
                continue;
        }
        d++;
    }
    if (mode != STRLIB_MODE_LEFT && e) { // for everything but trim_left, delete trailing matches.
        *e = 0;
    }
    return o;
}

char *trim(char *s) { return strcpytrim(s, s, STRLIB_MODE_BOTH, 0); }


int is_valid_rom(unsigned char *buffer) {
    /* Test if rom is a native .z64 image with header 0x80371240. [ABCD] */
    if((buffer[0]==0x80)&&(buffer[1]==0x37)&&(buffer[2]==0x12)&&(buffer[3]==0x40))
        return 0;
    /* Test if rom is a byteswapped .v64 image with header 0x37804012. [BADC] */
    else if((buffer[0]==0x37)&&(buffer[1]==0x80)&&(buffer[2]==0x40)&&(buffer[3]==0x12))
        return 1;
    /* Test if rom is a wordswapped .n64 image with header  0x40123780. [DCBA] */
    else if((buffer[0]==0x40)&&(buffer[1]==0x12)&&(buffer[2]==0x37)&&(buffer[3]==0x80))
        return 2;
    else
        return 0;
}

void swap_header(unsigned char* header, int loadlength) {
    unsigned char temp;
    int i;

    /* Btyeswap if .v64 image. */
    if( header[0]==0x37) {
        for (i = 0; i < loadlength; i+=2) {
            temp= header[i];
            header[i]= header[i+1];
            header[i+1]=temp;
            }
        }
    /* Wordswap if .n64 image. */
    else if( header[0]==0x40) {
        for (i = 0; i < loadlength; i+=4) {
            temp= header[i];
            header[i]= header[i+3];
            header[i+3]=temp;
            temp= header[i+1];
            header[i+1]= header[i+2];
            header[i+2]=temp;
        }
    }
}

void *memset(void *b, int c, int len)
{
  int           i;
  unsigned char *p = b;
  i = 0;
  while(len > 0)
    {
      *p = c;
      p++;
      len--;
    }
  return(b);
}

#define WAIT_ON_IOBUSY(stat)                                \
    stat = IO_READ(PI_STATUS_REG);                          \
    while (stat & (PI_STATUS_IO_BUSY | PI_STATUS_DMA_BUSY)) \
        stat = IO_READ(PI_STATUS_REG);


static __inline__ s32 rawdma(u32 devAddr, void *dramAddr, u32 size)
{
    register u32 stat;
    WAIT_ON_IOBUSY(stat);
    IO_WRITE(PI_DRAM_ADDR_REG, K0_TO_PHYS(dramAddr));
    IO_WRITE(PI_CART_ADDR_REG, K1_TO_PHYS((u32)osRomBase | devAddr));
    IO_WRITE(PI_WR_LEN_REG, size - 1);
    return 0;
}

extern FATFS loader_fs;
extern u8 _rebootSegmentStart[], _rebootSegmentRomStart[], _rebootSegmentRomEnd[];
void bootRom(void) {
    reboot_disable_interrupts();
    evd_setSaveType(SAVE_TYPE_EEP4k);
    evd_lockRegs();
    sleep(10);
    f_mount(0, "", 0);
    sleep(10);
    reboot_game(0);
}

// load a z64/v64/n64 rom to the sdram
void loadrom(u8 *buff) {

    TCHAR filename[64];
    sprintf(filename, "%s", buff);

    FRESULT result;
    FIL file;
    UINT bytesread = 0;
    result = f_open(&file, filename, FA_READ);

    if (result == FR_OK) {
        int swapped = 0;
        int headerfsize = 512;                 // rom-headersize 4096 but the bootcode is not needed
        unsigned char headerdata[headerfsize]; // 1*512
        int fsize = f_size(&file);
        int fsizeMB = fsize / 1048576; // Bytes in a MB

        result = f_read(&file,       /* [IN] File object */
                        headerdata,  /* [OUT] Buffer to store read data */
                        headerfsize, /* [IN] Number of bytes to read */
                        &bytesread   /* [OUT] Number of bytes read */
        );

        f_close(&file);

        int sw_type = is_valid_rom(headerdata);

        if (sw_type != 0) {
            swapped = 1;
            swap_header(headerdata, 512);
        }

        if (swapped == 1) {
            while (evd_isDmaBusy())
                ;
            evd_mmcSetDmaSwap(1);

            // TRACE(disp, "swapping on");
        }

        bytesread = 0;
        result = f_open(&file, filename, FA_READ);
        if (fsizeMB <= 32) {
            result = f_read(&file,               /* [IN] File object */
                            (void *) 0xb0000000, /* [OUT] Buffer to store read data */
                            fsize,               /* [IN] Number of bytes to read */
                            &bytesread           /* [OUT] Number of bytes read */
            );
        } else {
            result = f_read(&file,               /* [IN] File object */
                            (void *) 0xb0000000, /* [OUT] Buffer to store read data */
                            32 * 1048576,        /* [IN] Number of bytes to read */
                            &bytesread           /* [OUT] Number of bytes read */
            );
            if (result == FR_OK) {
                result = f_read(&file,               /* [IN] File object */
                                (void *) 0xb2000000, /* [OUT] Buffer to store read data */
                                fsize - bytesread,   /* [IN] Number of bytes to read */
                                &bytesread           /* [OUT] Number of bytes read */
                );
            }
        }

        // if (*(vu32*)0xB0000000 == 0x37804012) {
        //     vu32 *romarray = (vu32*) 0xB0000000;
        //     for (int i = 0; i < bytesread / 4; i++) {
        //         vu32 x = romarray[i];
        //         romarray[i] = ((x << 8) & 0xFF00FF00)
        //                     | ((x >> 8) & 0x00FF00FF);
        //     }
        // }

        // if (result == FR_OK) {
        //     bootRom(1);
        // }
    }
}