CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
CFLAGS=-mcpu=cortex-m7 -mthumb -Og -g -IInc
LDFLAGS=-TSTM32H750XBHX_FLASH.ld

SRC=Src/main.c Src/gpio.c Src/syscalls.c Src/sysmem.c Startup/startup_stm32h750xbhx.s
OUT=build

all: $(OUT).elf $(OUT).bin

$(OUT).elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

$(OUT).bin: $(OUT).elf
	$(OBJCOPY) -O binary $< $@

flash: $(OUT).bin
	st-flash write $(OUT).bin 0x08000000

debug: $(OUT).elf $(OUT).bin
	st-flash write $(OUT).bin 0x08000000
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
	rm -f $(OUT).elf $(OUT).bin

