CROSS_COMPILE := arm-linux-

CC := ${CROSS_COMPILE}gcc
AS := ${CROSS_COMPILE}as
LD := ${CROSS_COMPILE}ld
OBJCOPY := ${CROSS_COMPILE}objcopy
OBJDUMP := ${CROSS_COMPILE}objdump

CFLAGS := -mcpu=cortex-a8
CFLAGS += -mfpu=neon
ASFLAGS := -mcpu=cortex-a8
ASFLAGS += -mfpu=neon
LDFLAGS = -T u-boot-spl.lds -Map=$*.map
# only c source files go here
OBJS = dummy_app.o board.o

%.bin: %.elf
	@echo "converting .elf to .bin"
	${OBJCOPY} --gap-fill=0xFF -O binary $< $@

%.elf: %.o
	@echo "converting .o to .elf"
	${LD} ${LDFLAGS} $^ -o $@

%.o: %.c
	@echo "converting .c to .o"
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.s
	@echo "converting .s to .o"
	${AS} ${ASFLAGS} -c $< -o $@

#$(OBJS): timer.h bbb.h common.h debug.h interrupt.h arm.h leds.h serial.h
# since target OBJS is depending on all header files
# .o files will not be deleted


%_disasm: %.elf
	${OBJDUMP} -d $<

clean:
	@echo "cleaning"
	rm -f *.bin *.map *.elf *.o
