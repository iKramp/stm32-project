CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m7 -mthumb -Og -g -IInc
LDFLAGS = -TSTM32H750XBHX_FLASH.ld

SRC = Src/main.c Src/clock.c Src/gpio.c Src/register.c Src/syscalls.c Src/sysmem.c Startup/startup_stm32h750xbhx.s
OUT = build

ELF = $(OUT).elf
BIN = $(OUT).bin

all: $(BIN)

# Rule to build ELF from sources
$(ELF): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

# Rule to build BIN from ELF
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Flash target depends on the BIN file
flash: $(BIN)
	st-flash write $(BIN) 0x08000000

# Debug target also depends on BIN (ensures rebuild)
debug: $(BIN)
	st-flash write $(BIN) 0x08000000
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
	rm -f $(ELF) $(BIN)

