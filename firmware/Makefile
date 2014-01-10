#####################################################
# Makefile for FemtoTester                          #
#-------------------------                          #
# author: Markus Lohse <markus.lohse@bausch.com>    #
# date: 2013-05-10                                  #
#####################################################

# Target name:
TARGET = FemtoTester

# Fuses
HFUSE=0x99
LFUSE=0xEF

# Target type
MCU=atmega644a
PROGRAMMER=dragon_jtag

SRC =	main.c \
	uart.c \
	dock.c \
	timer.c \
	buffer.c \
	message.c \
	settings.c \
	protocol.c \
	footswitch.c \
	led.c

OBJ = $(SRC:.c=.o)


# Compiler / Linker flags:
CFLAGS = -mmcu=$(MCU) -Wall -g2 -O1 -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
LDFLAGS = -mmcu=$(MCU) -Wl,-Map=$(TARGET).map

###################################
# rules:

.PHONY: all program clean

all:	$(TARGET).hex $(TARGET).eep $(TARGET).lss

fuses:
	#avrdude -c $(PROGRAMMER) -P usb -p $(MCU) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m
	avrdude -c $(PROGRAMMER) -P usb -p atmega644 -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m # avrdude doesn't seem to know the ATMega644A yet

program:
	# avrdude doesn't seem to know atmega644a (yet), use atmega644 instead...
	# avrdude -c $(PROGRAMMER) -P usb -p $(MCU) -U flash:w:$(TARGET).hex
	avrdude -c $(PROGRAMMER) -P usb -p atmega644 -U flash:w:$(TARGET).hex
	sleep 2
	#avrdude -c $(PROGRAMMER) -P usb -p $(MCU) -U eeprom:w:$(TARGET).eep
	avrdude -c $(PROGRAMMER) -P usb -p atmega644 -U eeprom:w:$(TARGET).eep

clean:
	rm -f $(OBJ) $(TARGET).elf $(TARGET).hex $(TARGET).lss $(TARGET).map $(TARGET).eep


###################################
# Psudorules:

%.eep: %.elf
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

%.lss: %.elf
	avr-objdump -h -S $< > $@

%.elf: $(OBJ)
	avr-gcc $^ $(LDFLAGS) -o $@
	avr-size -C --mcu=$(MCU) $@

%.o : %.c
	avr-gcc $(CFLAGS) -c $< -o $@
