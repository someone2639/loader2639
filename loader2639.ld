OUTPUT_ARCH(mips)

#define BEGIN_SEG(name, addr)                                                                          \
    _##name##SegmentStart = ADDR(.name);                                                               \
    _##name##SegmentRomStart = __romPos;                                                               \
    .name addr: AT(__romPos)

#define END_SEG(name)                                                                                  \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name);                                                 \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name);                                                 \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name)                                                                             \
    _##name##SegmentBssStart = ADDR(.name.noload);                                                     \
    .name.noload(NOLOAD):

#define END_NOLOAD(name)                                                                               \
    _##name##SegmentBssEnd = ADDR(.name.noload) + SIZEOF(.name.noload);                                \
    _##name##SegmentBssSize = SIZEOF(.name.noload);

#define FILE(path)\
    path(.text*);\
    path(.data*);\
    path(.rodata*);

#define FILE_BSS(path)\
    path(COMMON);\
    path(.scommon*);\
    path(.bss);

SECTIONS {
    __romPos = 0;

    BEGIN_SEG(boot, 0x04000000) {
        BUILD_DIR/asm/rom_header.o(.text);
        BUILD_DIR/boot.6102.o(.data);
    }
    END_SEG(boot)

    BEGIN_SEG(code, 0x80000400) SUBALIGN(16) {
        BUILD_DIR/asm/entry.o(.text);
        BUILD_DIR/asm/tick.o(.text);

        BUILD_DIR/src/main.o(.text*);
        BUILD_DIR/src/main.o(.data*);
        BUILD_DIR/src/main.o(.rodata*);

        BUILD_DIR/src/allocator/allocator.o(.text*);

        FILE(BUILD_DIR/src/filesystem/ff.o)
        FILE(BUILD_DIR/src/filesystem/ffsystem.o)
        FILE(BUILD_DIR/src/filesystem/ffunicode.o)
        FILE(BUILD_DIR/src/filesystem/diskio.o)
        FILE(BUILD_DIR/src/sd_card/sd.o)
        FILE(BUILD_DIR/src/everdrive/everdrive.o)
        FILE(BUILD_DIR/src/io/mem.o)
        FILE(BUILD_DIR/src/io/sys.o)
        FILE(BUILD_DIR/src/game/boot.o)
        FILE(BUILD_DIR/src/game/crash_screen.o)
        FILE(BUILD_DIR/src/game/font.o)
        FILE(BUILD_DIR/src/math/sinf.o)
        FILE(BUILD_DIR/src/math/cosf.o)
        FILE(BUILD_DIR/src/game/loadrom.o)

        BUILD_DIR/libs2d_engine.a:*.o(.text*);
        BUILD_DIR/libs2d_engine.a:*.o(.data*);
        BUILD_DIR/libs2d_engine.a:*.o(.rodata*);

        */libultra_rom.a:*.o(.text*);
        */libultra_rom.a:*.o(.data*);
        */libultra_rom.a:*.o(.rodata*);

        /usr/lib/n64/PR/rspboot.o(.text*);
        /usr/lib/n64/PR/gspF3DEX2.fifo.o(.text*);
        /usr/lib/n64/PR/gspS2DEX2.fifo.o(.text*);

        /usr/lib/n64/PR/rspboot.o(.data*);
        /usr/lib/n64/PR/gspF3DEX2.fifo.o(.data*);
        /usr/lib/n64/PR/gspS2DEX2.fifo.o(.data*);

        */libgcc.a:*.o(.text*);
        */libgcc.a:*.o(.data*);
        */libgcc.a:*.o(.rodata*);
    }
    END_SEG(code)
    BEGIN_NOLOAD(code) {
        BUILD_DIR/src/main.o(.bss);

        BUILD_DIR/src/filesystem/ff.o(.bss*);
        BUILD_DIR/src/filesystem/ffsystem.o(.bss*);
        BUILD_DIR/src/filesystem/ffunicode.o(.bss*);
        BUILD_DIR/src/filesystem/diskio.o(.bss*);
        BUILD_DIR/src/sd_card/sd.o(.bss*);
        BUILD_DIR/src/everdrive/everdrive.o(.bss*);
        BUILD_DIR/src/io/mem.o(.bss*);
        BUILD_DIR/src/io/sys.o(.bss*);
        BUILD_DIR/src/game/boot.o(.bss*);
        BUILD_DIR/src/game/crash_screen.o(.bss*);

        BUILD_DIR/libs2d_engine.a:*.o(.bss*);

        */libultra_rom.a:*.o(COMMON);
        */libultra_rom.a:*.o(.scommon*);
        */libultra_rom.a:*.o(.bss*);

        */libgcc.a:*.o(COMMON);
        */libgcc.a:*.o(.scommon*);
        */libgcc.a:*.o(.bss*);
    }
    END_NOLOAD(code)

    _codeSegmentBssEnd = .;


    . = 0x80200000;
    BEGIN_NOLOAD(framebuffer){
        BUILD_DIR/src/buffers/framebuffer.o(.bss*);
    }
    END_NOLOAD(framebuffer)

    . = 0x80300000;
    BEGIN_NOLOAD(allocator){
        BUILD_DIR/src/allocator/allocator.o(.bss*);
    }
    END_NOLOAD(allocator)

    /* Discard everything not specifically mentioned above. */
    /DISCARD/ : {
        *(*);
    }
}
