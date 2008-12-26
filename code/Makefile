MCU = atmega8
# AVRLIB = /home/ushakov/devel/embedded/avrlib

ASFLAGS = -Wa, -gstabs
CPFLAGS	= -dr -g -Os -Wall -Wa,-ahlms=$(<:.c=.lst) -std=c99 -ffunction-sections -fdata-sections # -finline-functions-called-once 

CC	= avr-gcc
AS	= avr-gcc -x assembler-with-cpp	
RM	= rm -f
RN	= mv
CP	= cp
OBJCOPY	= avr-objcopy
OBJDUMP	= avr-objdump
SIZE	= avr-size
INCDIR	= .
FORMAT = ihex	

CPFLAGS += -mmcu=$(MCU)
ASFLAGS += -mmcu=$(MCU)


%.o : %.c 
	$(CC) -c $(CPFLAGS) -I$(INCDIR) $< -o $@

%.s : %.c
	$(CC) -S $(CPFLAGS) -I$(INCDIR) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) -I$(INCDIR) $< -o $@

%.elf:
	git-add $^
	git-commit -m "Autocommit when making $@" "*"
	egrep -h '^[a-zA-Z_0-9]+ +[a-zA-Z_0-9]+ ?\([a-zA-Z_0-9, *]*\);' $(patsubst %.c,%.h,$^) | sed 's|^|static |' > all-static.h
	cat $^ > all-code.c
	$(CC) all.c $(CPFLAGS) $(LIB) -Wl,-Map=$*.map,--cref -mmcu=$(MCU) -o $@
# was: $(CC) $^ $(LIB) -Wl,-Map=$*.map,--cref -mmcu=$(MCU) -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary -R .eeprom $< $@

%.hex: %.elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

%.eep: %.elf
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

%.cof: %.elf
	$(OBJCOPY) --debugging -O coff-ext-avr \
		--change-section-address   .data-0x800000 \
		--change-section-address    .bss-0x800000 \
		--change-section-address .noinit-0x800000 \
		--change-section-address .eeprom-0x810000 \
		$< $@

%.dmp: %.elf
	$(OBJDUMP) -h -S $< > $@


#################################################################
# Description of source files starts here

COMMON_SRC = flash.c delay.c put.c \
	beep.c uart.c eeprom.c time.c \
	dallas.c
#	dallas_comstub.c

TERMINAL_SRC = terminal.c term.c keyled.c team_numbers.c time_set.c led.c vt100.c
CPTEST_SRC = cptest.c led.c vt100.c
CPDEBUG_SRC = cpdebug.c filesys.c priority_queue.c led.c vt100.c
CPOINT_SRC = cpoint.c filesys.c cp.c priority_queue.c 

ALL_SRC = $(COMMON_SRC) $(TERMINAL_SRC) $(CPTEST_SRC) $(CPDEBUG_SRC) $(CPOINT_SRC)

TARGETS = terminal cptest cpdebug cpoint

default: cpoint

all: $(TARGETS)

cpdebug.elf: $(CPDEBUG_SRC:.c=.o) $(COMMON_SRC:.c=.o)

cpoint.elf: $(CPOINT_SRC) $(COMMON_SRC)
# cpoint.elf: $(CPOINT_SRC:.c=.o) $(COMMON_SRC:.c=.o)

cptest.elf: $(CPTEST_SRC:.c=.o) $(COMMON_SRC:.c=.o)

terminal.elf: $(TERMINAL_SRC:.c=.o) $(COMMON_SRC:.c=.o)

$(TARGETS): %: %.elf %.bin %.hex %.eep %.dmp
	$(SIZE) $*.elf
	@echo $< done

clean:
	$(RM) $(ALL_SRC:.c=.o)
	$(RM) $(ALL_SRC:.c=.s)
	$(RM) $(ALL_SRC:.c=.lst)
	$(RM) $(TARGETS:=.map)
	$(RM) $(TARGETS:=.elf)
	$(RM) $(TARGETS:=.cof)
	$(RM) $(TARGETS:=.obj)
	$(RM) $(TARGETS:=.a90)
	$(RM) $(TARGETS:=.sym)
	$(RM) $(TARGETS:=.eep)
	$(RM) $(TARGETS:=.hex)
	$(RM) $(TARGETS:=.bin)
	$(RM) *~
