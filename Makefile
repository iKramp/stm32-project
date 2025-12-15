CC      = arm-none-eabi-gcc
CXX     = arm-none-eabi-g++
AS      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

OUT = build

CFLAGS = \
	-mcpu=cortex-m7 \
	-mthumb \
	-Og \
	-g3 \
	-ffreestanding \
	-fdata-sections \
	-ffunction-sections \
	-IInc \
	-x c

CXXFLAGS = \
	$(CFLAGS) \
	-std=gnu++17 \
	-fno-exceptions \
	-fno-rtti \
	-fno-use-cxa-atexit \
	-fno-threadsafe-statics

LDFLAGS = \
	-mcpu=cortex-m7 \
	-mthumb \
	-TSTM32H750XBHX_FLASH.ld \
	-nostdlib \
	-Wl,--gc-sections \
	-Wl,-Map=$(OUT)/build.map \
	-lgcc \
	-lc \
	-lnosys

# ------------------------------------------------------------------------------

C_SRCS   := $(shell find Src -name '*.c')
CPP_SRCS := $(shell find Src -name '*.cpp')
ASM_SRCS := Startup/startup_stm32h750xbhx.s

C_OBJS   := $(patsubst %.c,$(OUT)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst %.cpp,$(OUT)/%.o,$(CPP_SRCS))
ASM_OBJS := $(patsubst %.s,$(OUT)/%.o,$(ASM_SRCS))

OBJS := $(C_OBJS) $(CPP_OBJS) $(ASM_OBJS)

ELF = $(OUT)/build.elf
BIN = $(OUT)/build.bin

# ------------------------------------------------------------------------------

all: $(BIN)

# Create output directories automatically
$(OUT)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(CFLAGS) -c $< -o $@

# Link
$(ELF): $(OBJS)
	@mkdir -p $(OUT)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# ------------------------------------------------------------------------------

flash: $(BIN)
	st-flash write $(BIN) 0x08000000

debug: $(BIN)
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

clean:
	rm -rf $(OUT)

