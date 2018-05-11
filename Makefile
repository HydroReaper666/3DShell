#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#
# NO_SMDH: if set to anything, no SMDH file is generated.
# ROMFS is the directory which contains the RomFS, relative to the Makefile (Optional)
# APP_TITLE is the name of the app stored in the SMDH file (Optional)
# APP_DESCRIPTION is the description of the app stored in the SMDH file (Optional)
# APP_AUTHOR is the author of the app stored in the SMDH file (Optional)
# ICON is the filename of the icon (.png), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.png
#     - icon.png
#     - <libctru folder>/default_icon.png
#---------------------------------------------------------------------------------

# CIA
APP_TITLE           := 3DShell
APP_PRODUCT_CODE    := CTR-3D-SHEL
APP_ROMFS           := romfs
APP_CATEGORY        := Application
APP_UNIQUE_ID       := 0x16200
APP_USE_ON_SD       := true
APP_ENCRYPTED       := false
APP_MEMORY_TYPE     := Application
APP_SYSTEM_MODE     := 64MB
APP_SYSTEM_MODE_EXT := Legacy
APP_CPU_SPEED       := 268MHz
APP_ENABLE_L2_CACHE := true
VERSION_MAJOR       := 3
VERSION_MINOR       := 0
VERSION_MICRO       := 0
APP_RSF_FILE        := resources/cia.rsf

APP_DESCRIPTION := Multi purpose file manager GUI
APP_AUTHOR      := Joel16

TARGET      := $(subst $e ,_,$(notdir $(APP_TITLE)))
OUTDIR      := out
BUILD       := build
SOURCES     := source source/audio source/ftp source/menus source/minizip source/misc source/pp2d
DATA        := data
INCLUDES    := include include/audio include/dr_libs include/ftp include/menus include/minizip include/misc include/pp2d

ICON        := resources/ic_launcher_filemanager.png
BANNER      := resources/banner.png
BANNER_AUDIO      := resources/banner.wav
LOGO        := resources/logo.lz11
ICON_FLAGS  := nosavebackups,visible

GITVERSION    := $(shell git log -1 --pretty='%h')

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS   := -g -Werror -O2 -mword-relocations \
	        -fomit-frame-pointer -ffunction-sections \
	        -DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MICRO=$(VERSION_MICRO) \
	        -DGITVERSION="\"${GITVERSION}\"" \
            $(ARCH)

CFLAGS   += $(INCLUDE) -DARM11 -D_3DS

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS  := -g $(ARCH)
LDFLAGS  = -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lmpg123 -lvorbisidec -logg -lcitro3d -lctru -lm -lz

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB) $(PORTLIBS)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT := $(CURDIR)/$(OUTDIR)/$(TARGET)
export TOPDIR := $(CURDIR)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
				$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES	:= $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD := $(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD := $(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES := $(addsuffix .o,$(BINFILES)) \
			$(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(OUTPUT).smdh
endif

ifneq ("$(wildcard $(APP_ROMFS))","")
    _3DSXFLAGS += --romfs=$(APP_ROMFS)
endif

ifeq ($(strip $(VERSION_MAJOR)),)
    VERSION_MAJOR := 0
endif

ifeq ($(strip $(VERSION_MINOR)),)
    VERSION_MINOR := 0
endif

ifeq ($(strip $(VERSION_MICRO)),)
    VERSION_MICRO := 0
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: 3dsx cia

3dsx: $(BUILD) $(OUTPUT).3dsx

cia : $(BUILD) $(OUTPUT).cia

citra: export CITRA_MODE = 1
citra: 3dsx
#---------------------------------------------------------------------------------
$(BUILD):
	@mkdir -p $(OUTDIR)
	@[ -d "$@" ] || mkdir -p "$@"
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTDIR)

#---------------------------------------------------------------------------------
ifeq ($(strip $(NO_SMDH)),)
$(OUTPUT).3dsx : $(OUTPUT).elf $(OUTPUT).smdh
else
$(OUTPUT).3dsx : $(OUTPUT).elf
endif

#---------------------------------------------------------------------------------
MAKEROM      ?= makerom
MAKEROM_ARGS := -rsf "$(APP_RSF_FILE)" -target t -exefslogo -icon "$(BUILD)/icon.icn" -banner "$(BUILD)/banner.bnr" -major $(VERSION_MAJOR) -minor $(VERSION_MINOR) -micro $(VERSION_MICRO) -DAPP_TITLE="$(APP_TITLE)" -DAPP_PRODUCT_CODE="$(APP_PRODUCT_CODE)" -DAPP_UNIQUE_ID="$(APP_UNIQUE_ID)" -DAPP_SYSTEM_MODE="$(APP_SYSTEM_MODE)" -DAPP_SYSTEM_MODE_EXT="$(APP_SYSTEM_MODE_EXT)" -DAPP_CATEGORY="$(APP_CATEGORY)" -DAPP_USE_ON_SD="$(APP_USE_ON_SD)" -DAPP_MEMORY_TYPE="$(APP_MEMORY_TYPE)" -DAPP_CPU_SPEED="$(APP_CPU_SPEED)" -DAPP_ENABLE_L2_CACHE="$(APP_ENABLE_L2_CACHE)" -DAPP_VERSION_MAJOR=$(VERSION_MAJOR)

ifneq ("$(wildcard $(LOGO))","")
    MAKEROM_ARGS += -logo "$(LOGO)"
endif

ifneq ("$(wildcard $(APP_ROMFS))","")
    MAKEROM_ARGS += -DAPP_ROMFS="$(APP_ROMFS)"
endif

$(OUTPUT).cia: $(OUTPUT).elf $(BUILD)/banner.bnr $(BUILD)/icon.icn 
	$(MAKEROM) -f cia -o $@ -elf $< -DAPP_ENCRYPTED=false $(MAKEROM_ARGS)

BANNERTOOL ?= bannertool

ifeq ($(suffix $(BANNER)),.cgfx)
	BANNER_ARG := -ci
else
	BANNER_ARG := -i
endif

ifeq ($(suffix $(BANNER_AUDIO)),.cwav)
	BANNER_AUDIO_ARG := -ca
else
	BANNER_AUDIO_ARG := -a
endif

$(BUILD)/banner.bnr : $(BANNER) $(BANNER_AUDIO)
	$(BANNERTOOL) makebanner $(BANNER_ARG) "$(BANNER)" $(BANNER_AUDIO_ARG) "$(BANNER_AUDIO)" -o "$@"

$(BUILD)/icon.icn : $(APP_ICON)
	$(BANNERTOOL) makesmdh -s "$(APP_TITLE)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i "$(APP_ICON)" -f "$(ICON_FLAGS)" -o "$@"

#---------------------------------------------------------------------------------
else

DEPENDS := $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------

$(OUTPUT).elf : $(OFILES)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval CURBIN := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $@
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
endef

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
