STMCC       = arm-none-eabi-gcc
STMCXX      = arm-none-eabi-g++
STMAS       = arm-none-eabi-gcc
STMOBJCOPY  = arm-none-eabi-objcopy

HOSTCC      = gcc
HOSTCXX     = g++
HOSTAS      = as
HOSTOBJCOPY = objcopy

# ==============================================================================
# FLAGS
# ==============================================================================

COMMON_INC = \
	-Iinclude \
	-Icore

STMCFLAGS = \
	-mcpu=cortex-m7 \
	-mthumb \
	-O0 \
	-g3 \
	-ffreestanding \
	-fdata-sections \
	-ffunction-sections \
	$(COMMON_INC)

STMCXXFLAGS = \
	$(STMCFLAGS) \
	-std=gnu++17 \
	-fno-exceptions \
	-fno-rtti \
	-fno-use-cxa-atexit \
	-fno-threadsafe-statics

STMLDFLAGS = \
	-mcpu=cortex-m7 \
	-mthumb \
	-TSTM32H750XBHX_FLASH.ld \
	-nostdlib \
	-Wl,--gc-sections \
	-Wl,-Map=$(STMOUT)/build.map \
	-lgcc \
	-lnosys

HOSTCFLAGS = \
	-O2 \
	-g \
	$(COMMON_INC)

HOSTCXXFLAGS = \
	$(HOSTCFLAGS) \
	-std=gnu++17

HOSTLDFLAGS =

STMOUT_SERVER = stm_build_server
STMOUT_CLIENT = stm_build_client
HOSTOUT_SERVER = host_build_server
HOSTOUT_CLIENT = host_build_client

ifeq ($(SERVER),1)
    STMCFLAGS += -DSERVER_BUILD
	HOSTCFLAGS += -DSERVER_BUILD

	STMOUT      = $(STMOUT_SERVER)
	HOSTOUT     = $(HOSTOUT_SERVER)
else
	STMOUT      = $(STMOUT_CLIENT)
	HOSTOUT     = $(HOSTOUT_CLIENT)
endif

# ==============================================================================
# SOURCES
# ==============================================================================

ASM_SRCS := Startup/startup_stm32h750xbhx.s

CORE_C_SRCS   := $(shell find core -name '*.c')
CORE_CPP_SRCS := $(shell find core -name '*.cpp')

STM_C_SRCS    := $(shell find platform/stm32 -name '*.c')
STM_CPP_SRCS  := $(shell find platform/stm32 -name '*.cpp')

HOST_C_SRCS   := $(shell find platform/linux -name '*.c')
HOST_CPP_SRCS := $(shell find platform/linux -name '*.cpp')

# ==============================================================================
# OBJECTS
# ==============================================================================

STM_ASM_OBJS := $(patsubst %.s,$(STMOUT)/%.o,$(ASM_SRCS))

STM_CORE_C_OBJS   := $(patsubst %.c,$(STMOUT)/%.o,$(CORE_C_SRCS))
STM_CORE_CPP_OBJS := $(patsubst %.cpp,$(STMOUT)/%.o,$(CORE_CPP_SRCS))

HOST_CORE_C_OBJS   := $(patsubst %.c,$(HOSTOUT)/%.o,$(CORE_C_SRCS))
HOST_CORE_CPP_OBJS := $(patsubst %.cpp,$(HOSTOUT)/%.o,$(CORE_CPP_SRCS))

STM_C_OBJS    := $(patsubst %.c,$(STMOUT)/%.o,$(STM_C_SRCS))
STM_CPP_OBJS  := $(patsubst %.cpp,$(STMOUT)/%.o,$(STM_CPP_SRCS))

HOST_C_OBJS   := $(patsubst %.c,$(HOSTOUT)/%.o,$(HOST_C_SRCS))
HOST_CPP_OBJS := $(patsubst %.cpp,$(HOSTOUT)/%.o,$(HOST_CPP_SRCS))

STM_OBJS = \
	$(STM_ASM_OBJS) \
	$(STM_CORE_C_OBJS) \
	$(STM_CORE_CPP_OBJS) \
	$(STM_C_OBJS) \
	$(STM_CPP_OBJS)

HOST_OBJS = \
	$(HOST_CORE_C_OBJS) \
	$(HOST_CORE_CPP_OBJS) \
	$(HOST_C_OBJS) \
	$(HOST_CPP_OBJS)

# ==============================================================================
# OUTPUTS
# ==============================================================================

STMELF = $(STMOUT)/build.elf
STMBIN = $(STMOUT)/build.bin

HOSTELF = $(HOSTOUT)/host.elf

# ==============================================================================
# DEFAULT TARGET
# ==============================================================================

all: $(STMBIN) $(HOSTELF)

# ==============================================================================
# STM32 BUILD RULES
# ==============================================================================

$(STMOUT)/%.o: %.c
	@mkdir -p $(dir $@)
	$(STMCC) $(STMCFLAGS) -c $< -o $@

$(STMOUT)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(STMCXX) $(STMCXXFLAGS) -c $< -o $@

$(STMOUT)/%.o: %.s
	@mkdir -p $(dir $@)
	$(STMAS) $(STMCFLAGS) -c $< -o $@

# ==============================================================================
# HOST BUILD RULES
# ==============================================================================

$(HOSTOUT)/%.o: %.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTCFLAGS) -c $< -o $@

$(HOSTOUT)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(HOSTCXX) $(HOSTCXXFLAGS) -c $< -o $@

# ==============================================================================
# LINKING
# ==============================================================================

$(STMELF): $(STM_OBJS)
	@mkdir -p $(STMOUT)
	$(STMCXX) $(STM_OBJS) -o $@ $(STMLDFLAGS)

$(HOSTELF): $(HOST_OBJS)
	@mkdir -p $(HOSTOUT)
	$(HOSTCXX) $(HOST_OBJS) -o $@ $(HOSTLDFLAGS)

# ==============================================================================
# BIN
# ==============================================================================

$(STMBIN): $(STMELF)
	$(STMOBJCOPY) -O binary $< $@

# ==============================================================================
# UTILITIES
# ==============================================================================

flash: $(STMBIN)
	st-flash write $(STMBIN) 0x08000000

debug: $(STMBIN)
	@bash -c '\
		set -m; \
		openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "tcl_port 6667" > /dev/null 2>&1 & \
		OCD_PID=$$!; \
		sleep 1; \
		gdb -x gdb_commands.txt; \
		kill $$OCD_PID 2>/dev/null || true; \
		wait $$OCD_PID 2>/dev/null || true; \
	'

rescue:
	st-flash --connect-under-reset --freq=1000 erase

host: $(HOSTELF)
	./$(HOSTELF)

clean:
	rm -rf $(STMOUT_SERVER)
	rm -rf $(STMOUT_CLIENT)
	rm -rf $(HOSTOUT_SERVER)
	rm -rf $(HOSTOUT_CLIENT)

.PHONY: all clean flash debug rescue host
