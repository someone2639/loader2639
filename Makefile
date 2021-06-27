#! smake
#---------------------------------------------------------------------
#	Copyright (C) 1997, Nintendo.
#
#	File		Makefile
#	Coded    by	Yoshitaka Yasumoto.	Mar 19, 1997.
#	Modified by
#	Comments
#
#	$Id: Makefile,v 1.17 1999/04/02 10:10:55 yoshida Exp $
#---------------------------------------------------------------------
ROMNAME = loader2639

BUILD_DIR = build

include /usr/include/n64/make/PRdefs

OPTIMIZER       = -O2
LCDEFS          = -DNDEBUG -D_FINALROM
N64LIB          = -lultra_rom

ELF		= $(BUILD_DIR)/$(ROMNAME).elf
GAME	= $(BUILD_DIR)/$(ROMNAME).z64
MAP		= $(BUILD_DIR)/$(ROMNAME).map

LD_SCRIPT	= $(ROMNAME).ld
CP_LD_SCRIPT	= $(BUILD_DIR)/$(ROMNAME).ld

ASMFILES    =	asm/entry.s asm/rom_header.s
ASMOBJECTS  =	$(BUILD_DIR)/$(ASMFILES:.s=.o)

BOOT		= /usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	= $(BUILD_DIR)/boot.6102.o


ASM_DIRS = asm
SRC_DIRS = src src/buffers
ALL_BUILD_DIRS = $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(ASM_DIRS) $(SRC_DIRS))
_ != mkdir -p $(BUILD_DIR) $(ALL_BUILD_DIRS)

C_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES = $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))


OBJECTS = $(foreach fl, $(C_FILES), $(BUILD_DIR)/$(fl:.c=.o)) \
		  $(foreach fl, $(S_FILES), $(BUILD_DIR)/$(fl:.s=.o)) \
		  $(BOOT_OBJ)

LCINCS  =	-I. -I/usr/include/n64/PR
LCOPTS =	-G 0

LCDEFS  +=	-DF3DEX_GBI -DF3DEX_GBI_2 -DS2DEX_GBI_2

LDIRT  =	$(ELF) $(CP_LD_SCRIPT) $(MAP) $(ASMOBJECTS) $(GAME)
		    
LDFLAGS =	-L/usr/lib/n64 $(N64LIB) -L$(N64_LIBGCCDIR) -lgcc

default:	$(GAME)

include $(COMMONRULES)

$(BUILD_DIR)/%.o: %.s
	$(AS) -Wa,-Iasm -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

test: $(GAME)
	~/Downloads/mupen64plus/mupen64plus-gui $<
clean:
	rm -r $(BUILD_DIR)

$(BOOT_OBJ): $(BOOT)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

$(CP_LD_SCRIPT): $(LD_SCRIPT)
	cpp -P -Wno-trigraphs -DBUILD_DIR=$(BUILD_DIR) -o $@ $<

$(GAME): $(OBJECTS) $(CP_LD_SCRIPT)
	$(LD) -L. -T $(CP_LD_SCRIPT) -Map $(MAP) -o $(ELF) $(LDFLAGS)
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(ELF) $(GAME) -O binary
	makemask $(GAME)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
