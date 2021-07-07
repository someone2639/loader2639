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

        // char 32-51 name
        unsigned char rom_name[32];

        for (int u = 0; u < 19; u++) {
            if (u != 0)
                sprintf(rom_name, "%s%c", rom_name, headerdata[32 + u]);
            else
                sprintf(rom_name, "%c", headerdata[32 + u]);
        }

        // rom name
        sprintf(rom_name, "%s", trim(rom_name));

        // rom size
        sprintf(rom_name, "Size: %iMB", fsizeMB);

        // unique cart id for gametype
        unsigned char cartID_str[12];
        sprintf(cartID_str, "ID: %c%c%c%c", headerdata[0x3B], headerdata[0x3C], headerdata[0x3D],
                headerdata[0x3E]);

        int cic, save;

        // cic = get_cic(&headerdata[0x40]);

        unsigned char cartID_short[4];
        sprintf(cartID_short, "%c%c\0", headerdata[0x3C], headerdata[0x3D]);

        
        // new rom_config
        // boot_cic = rom_config[1] + 1;
        // boot_save = rom_config[2];
        // force_tv = rom_config[3];
        // cheats_on = rom_config[4];
        // checksum_fix_on = rom_config[5];
        // boot_country = rom_config[7]; // boot_block


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

        if (result == FR_OK) {
            bootRom(1);
        }
    }
}