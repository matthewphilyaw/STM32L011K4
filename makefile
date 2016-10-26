# Project Setup
PROJECT=STM32L0
OUT_DIR=build
TOOL_ROOT=/usr/local/gcc-arm-none-eabi-5_2-2015q4/bin

DEFS=-DSTM32L011xx -DUSE_FULL_LL_DRIVER
MCU=cortex-m0plus

SYS_DIR=system

# Src/Include dirs
INC_DIRS=include \
		 $(SYS_DIR)/include \
		 $(SYS_DIR)/include/cmsis \
		 $(SYS_DIR)/include/cmsis/device \
		 $(SYS_DIR)/include/STM32L0xx

USR_SRC_DIR=src
SYS_SRC_DIR=$(SYS_DIR)/src

# Output Files
ELF=$(PROJECT).elf
HEX=$(PROJECT).hex
BIN=$(PROJECT).bin
LST=$(PROJECT).lst
MAP=$(PROJECT).map

# Commands
MKDIR_P=mkdir -p

# Tool Chain
PREFIX=arm-none-eabi-
TOOL=$(TOOL_ROOT)/$(PREFIX)
CC=$(TOOL)gcc
CXX=$(TOOL)g++
LD=$(TOOL)ld
AR=$(TOOL)ar
AS=$(TOO)as
CP=$(TOOL)objcopy
OD=$(TOOL)objdump
NM=$(TOOL)nm
SIZE=$(TOOL)size
A2L=$(TOOL)addr2line

# Gather Files

INCS=$(INC_DIRS:%=-I%)

LDSCRIPTS_FILES=$(shell find -L $(SYS_DIR) -name '*.ld')
LDSCRIPTS=$(LDSCRIPTS_FILES:%=-T%)

# System files
SYS_ASM=$(shell find -L $(SYS_SRC_DIR) -name '*.S')
SYS_C=$(shell find -L $(SYS_SRC_DIR) -name '*.c')
SYS_CXX=$(shell find -L $(SYS_SRC_DIR) -name '*.cpp')

SYS_OBJS=$(SYS_ASM:%.S=$(OUT_DIR)/%.o)
SYS_OBJS+=$(SYS_C:%.c=$(OUT_DIR)/%.o)
SYS_OBJS+=$(SYS_CXX:%.cpp=$(OUT_DIR)/%.o)

# User files
USR_ASMS=$(shell find -L $(USR_SRC_DIR) -name '*.S')
USR_C=$(shell find -L $(USR_SRC_DIR) -name '*.c')
USR_CXX=$(shell find -L $(USR_SRC_DIR) -name '*.cpp')

USR_OBJS=$(USR_ASM:%.S=$(OUT_DIR)/%.o)
USR_OBJS+=$(USR_C:%.c=$(OUT_DIR)/%.o)
USR_OBJS+=$(USR_CXX:%.cpp=$(OUT_DIR)/%.o)

OBJS=$(SYS_OBJS)
OBJS+=$(USR_OBJS)

OBJ_OUT_DIRS=$(subst /,/,$(sort $(dir $(OBJS))))

MCU_FLAGS=-mcpu=$(MCU) -mthumb -mfloat-abi=soft

BASE_FLAGS=$(MCU_FLAGS) $(DEFS) $(INCS)
BASE_FLAGS+=-Wall \
			-fmessage-length=0 \
			-ffunction-sections \
			-c

C_FLAGS=$(BASE_FLAGS)

CXX_FLAGS=$(BASE_FLAGS) \
		  -std=c++0x \
		  -fno-exceptions \
		  -fno-rtti \
		  -fno-threadsafe-statics \
		  -Wextra

LD_FLAGS=$(LDSCRIPTS) \
		 $(MCU_FLAGS) \
		 -fno-exceptions \
		 -fno-rtti \
		 -Wl,-Map=$(OUT_DIR)/$(MAP) \
		 -Wl,--gc-sections \
		 --specs=nano.specs \
		 -lm

.PHONY: all release release-memopt debug debug-no-opt clean clean-all

all: dirs debug

dirs: $(OBJ_OUT_DIRS) 

$(OBJ_OUT_DIRS):
	@echo $(INCS)
	$(MKDIR_P) $(OBJ_OUT_DIRS)

release-memopt: C_FLAGS+=-O2
release-memopt: CXX_FLAGS+=-O2
release-memopt: LD_FLAGS+=-O2
release-memopt: release

release-small: C_FLAGS+=-Os
release-small: CXX_FLAGS+=-Os
release-small: LD_FLAGS+=-Os
release-small: release

debug: DEFS+=-DDEBUG
debug: C_FLAGS+=-Og -g3
debug: CXX_FLAGS+=-Og -g3
debug: LD_FLAGS+=-g3
debug: release

debug-no-opt: DEFS+=-DDEBUG
debug-no-opt: C_FLAGS+=-O0 -g3
debug-no-opt: CXX_FLAGS+=-O0 -g3
debug-no-opt: LD_FLAGS+=-g0
debug-no-opt: release

release: $(OUT_DIR)/$(LST)
release: $(OUT_DIR)/$(HEX)
release: $(OUT_DIR)/$(BIN)

$(OUT_DIR)/$(BIN): $(OUT_DIR)/$(ELF)
	$(CP) -S -O binary $< $@
	@echo "Objcopy from ELF to bin complete!\n"

$(OUT_DIR)/$(HEX): $(OUT_DIR)/$(ELF)
	$(CP) -O ihex $< $@
	@echo "Objcopy from ELF to IHEX complete!\n"

$(OUT_DIR)/$(LST): $(OUT_DIR)/$(ELF)
	$(OD) -D -S $< > $(OUT_DIR)/$(LST)
	@echo "Objdump from ELF to listing complete!\n"

$(OUT_DIR)/$(ELF): $(OBJS)
	$(CXX) -o $@ $(LD_FLAGS) $(OBJS)
	@echo "Linking complete!\n"
	$(SIZE) $(OBJS) $(OUT_DIR)/$(ELF)

$(OUT_DIR)/%.o: %.cpp
	$(CXX) $(CXX_FLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

$(OUT_DIR)/%.o: %.c
	$(CC) $(C_FLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

$(OUT_DIR)/%.o: %.S
	$(CC) $(C_FLAGS) $< -o $@
	@echo "Assambled "$<"!\n"

clean:
	rm -f \
		$(USR_OBJS) \
		$(USR_OBJS:.o=.d) \
		$(OUT_DIR)/$(ELF) \
		$(OUT_DIR)/$(HEX) \
		$(OUT_DIR)/$(LST) \
		$(OUT_DIR)/$(BIN) \
		$(OUT_DIR)/$(MAP)

clean-all:
	rm -f \
		$(OBJS) \
		$(OBJS:.o=.d) \
		$(OUT_DIR)/$(ELF) \
		$(OUT_DIR)/$(HEX) \
		$(OUT_DIR)/$(LST) \
		$(OUT_DIR)/$(BIN) \
		$(OUT_DIR)/$(MAP)
