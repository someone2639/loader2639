OUTPUT_ARCH (mips)

#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name addr : AT(__romPos)

#define END_SEG(name) \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name) \
    _##name##SegmentBssStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

#define END_NOLOAD(name) \
    _##name##SegmentBssEnd = ADDR(.name.noload) + SIZEOF(.name.noload); \
    _##name##SegmentBssSize = SIZEOF(.name.noload);

SECTIONS
{
   __romPos = 0;

   BEGIN_SEG(boot, 0x04000000)
   {
      BUILD_DIR/asm/rom_header.o(.text);
      BUILD_DIR/boot.6102.o(.data);
   }
   END_SEG(boot)

   BEGIN_SEG(code, 0x80000400) SUBALIGN(16)
   {
      BUILD_DIR/asm/entry.o(.text);

      BUILD_DIR/src/main.o(.text*);


      /usr/lib/n64/libultra_rom.a:*.o(.text*);
      /usr/lib/n64/PR/rspboot.o(.text);
      /usr/lib/n64/PR/gspS2DEX2.fifo.o(.text);

      /usr/lib/n64/libultra_rom.a:*.o(.data*);
      /usr/lib/n64/PR/rspboot.o(.data*);
      /usr/lib/n64/PR/gspS2DEX2.fifo.o(.data*);

      /usr/lib/n64/libultra_rom.a:*.o(.rodata*);
   }
   END_SEG(code)
   BEGIN_NOLOAD(code)
   {
      BUILD_DIR/src/main.o(.bss);
      /usr/lib/n64/libultra_rom.a:*.o(COMMON);
      /usr/lib/n64/libultra_rom.a:*.o(.scommon*);
      /usr/lib/n64/libultra_rom.a:*.o(.bss*);
   }
   END_NOLOAD(code)
   
   _codeSegmentBssEnd = .;
   
   /* Discard everything not specifically mentioned above. */
   /DISCARD/ :
   {
      *(*);
   }
}
