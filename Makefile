ROMNAME = loader2639

BUILD_DIR = build

include /usr/include/n64/make/PRdefs

OPTIMIZER       = -O2
LCDEFS          = -DNDEBUG -D_FINALROM
N64LIB          = -lultra_rom -ls2d_engine

ELF		= $(BUILD_DIR)/$(ROMNAME).elf
GAME	= $(BUILD_DIR)/$(ROMNAME).z64
MAP		= $(BUILD_DIR)/$(ROMNAME).map

LD_SCRIPT	= $(ROMNAME).ld
CP_LD_SCRIPT	= $(BUILD_DIR)/$(ROMNAME).ld

ASMFILES    =	asm/entry.s asm/rom_header.s
ASMOBJECTS  =	$(BUILD_DIR)/$(ASMFILES:.s=.o)

BOOT		= /usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	= $(BUILD_DIR)/boot.6102.o


ASM_DIRS = asm cringe
ASSET_DIRS = assets/crashscreen_font
SRC_DIRS = src src/game src/buffers src/allocator src/filesystem \
           src/sd_card src/everdrive src/io src/math src/lib
ALL_BUILD_DIRS = $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(ASM_DIRS) $(SRC_DIRS) $(ASSET_DIRS))
_ != mkdir -p $(ALL_BUILD_DIRS)
_ != make -C tools
_ != make -C src/s2d_engine COPY_DIR=../../build/ CROSS=mips-n64-

C_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES = $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))


OBJECTS = $(foreach fl, $(C_FILES), $(BUILD_DIR)/$(fl:.c=.o)) \
		  $(foreach fl, $(S_FILES), $(BUILD_DIR)/$(fl:.s=.o)) \
		  $(BOOT_OBJ)

LCINCS  =	-I. -I/usr/include/n64/PR -Iinclude/ -Isrc/ -Ibuild/
LCOPTS =	-G 0

LCDEFS  +=	-DF3DEX_GBI -DF3DEX_GBI_2 -DS2DEX_GBI_2

LDIRT  =	$(ELF) $(CP_LD_SCRIPT) $(MAP) $(ASMOBJECTS) $(GAME)
		    
LDFLAGS =	-L/usr/lib/n64 $(N64LIB) -L$(N64_LIBGCCDIR) -lgcc

default:	$(GAME)

include $(COMMONRULES)

# n64graphics rules

# TEXTURES = $(foreach dir,$(ASSET_DIRS),$(wildcard $(dir)/*.png))
# TEXTURE_INC = $(foreach fl, $(TEXTURES), $(BUILD_DIR)/$(fl:.png=.inc.c))

# $(BUILD_DIR)/%: %.png
# 	$(call print,Converting:,$<,$@)
# 	tools/n64graphics -s raw -i $@ -g $< -f $(lastword $(subst ., ,$@))

# $(BUILD_DIR)/%.inc.c: %.png
# 	$(call print,Converting:,$<,$@)
# 	tools/n64graphics -s u8 -i $@ -g $< -f $(lastword $(subst ., ,$(basename $<)))

# $(BUILD_DIR)/src/game/crash_screen.o: $(TEXTURE_INC)

CFLAGS = -c -mabi=32 -ffreestanding -mfix4300 $(LCOPTS) \
         -DNDEBUG -D_FINALROM $(LCDEFS) \
         $(LCINCS) -I/usr/include/n64 -g

$(BUILD_DIR)/%.o: %.s
	$(AS) -Wa,-Iasm -Wa,-Icringe -Wa,-Iinclude -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

test: $(GAME)
	~/Devel/m64p/mupen64plus/mupen64plus-gui $<
test-pj64: $(GAME)
	wine ~/Desktop/new64/Project64.exe $<
clean:
	rm -r $(BUILD_DIR)
	make -C src/s2d_engine clean
load: $(GAME)
	cp $< /run/media/faris/n64/ED64P/OS64P.V64

$(BOOT_OBJ): $(BOOT)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

$(CP_LD_SCRIPT): $(LD_SCRIPT)
	cpp -P -Wno-trigraphs -DBUILD_DIR=$(BUILD_DIR) -o $@ $<

$(GAME): $(OBJECTS) $(TEXTURE_INC) $(CP_LD_SCRIPT)
	$(LD) -L. -Lbuild -T $(CP_LD_SCRIPT) -Map $(MAP) -o $(ELF) $(LDFLAGS)
	$(OBJCOPY) --pad-to=0x200000 --gap-fill=0xFF $(ELF) $(GAME) -O binary
	makemask $(GAME)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
