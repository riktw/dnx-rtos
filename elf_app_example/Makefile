CROSS?=arm-none-eabi-
CC=$(CROSS)gcc
LD=$(CROSS)gcc
STRIP=$(CROSS)strip
SIZE=$(CROSS)size

SRC=main.c start.c

OPT?=0

CFLAGS=-mcpu=cortex-m3 -mthumb -O$(OPT) -ggdb3 -I../src/system/include \
	 -I../src/system/include/libc -I../config \
	 -I../src/system/kernel/FreeRTOS/Source/include \
	 -I../src/system/kernel/FreeRTOS \
	 -I../src/system/cpu \
	 -I../src/system/kernel/FreeRTOS/Source/portable/GCC/RL78 \
	 -I../build \
	 -I../src/system \
	 -I../src/system/drivers/afm \
	 -I../src/system/drivers/can \
	 -I../src/system/drivers/clk \
	 -I../src/system/drivers/crc \
	 -I../src/system/drivers/dci \
	 -I../src/system/drivers/dht11 \
	 -I../src/system/drivers/dma \
	 -I../src/system/drivers/eth \
	 -I../src/system/drivers/fmc \
	 -I../src/system/drivers/gpio \
	 -I../src/system/drivers/i2c \
	 -I../src/system/drivers/i2cee \
	 -I../src/system/drivers/irq \
	 -I../src/system/drivers/loop \
	 -I../src/system/drivers/nvm \
	 -I../src/system/drivers/pwm \
	 -I../src/system/drivers/rtc \
	 -I../src/system/drivers/sdspi \
	 -I../src/system/drivers/tty \
	 -I../src/system/drivers/sdio \
	 -I../src/system/drivers/snd \
	 -I../src/system/drivers/spi \
	 -I../src/system/drivers/spiee \
	 -I../src/system/drivers/uart \
	 -I../src/system/drivers/usbd \
	 -I../src/system/drivers/wdg \
	 -I../src/system/cpu/stm32f4/stm32f4xx \
	 -I../src/system/cpu/lib/CMSIS \
	-mword-relocations -mlong-calls -fno-common \

LDFLAGS=-r -Bsymbolic -nostartfiles \
	-mcpu=cortex-m3 -mthumb -mlong-calls -fno-common \
	-T elf.ld

OBJS=$(SRC:.c=.o)
DEPS=$(SRC:.c=.d)

all: app.elf

-include $(DEPS)

%.o: %.c
	@echo " CC $<"
	@$(CC) -MMD $(CFLAGS) -o $@ -c $<

app.elf: $(OBJS)
	@echo " LINK $@"
	@$(LD) $(LDFLAGS) -o $@ $^
	@$(STRIP) -g -o app-striped.elf $@
	@$(SIZE) --common $@

.PHONY: clean all list

clean:
	@echo " CLEAN"
	@rm -fR $(OBJS) $(DEPS) *.elf

list:
	@echo " Creating list..."
	@$(CROSS)objdump -hpxdSsrt app.elf > app.elf.list
	@$(CROSS)readelf -atSln app.elf > app.elf.read
