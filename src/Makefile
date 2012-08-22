PROG = beer_counter

all: $(PROG).hex

CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -mmcu=atmega8 -Wall -O2 -fshort-enums -ffreestanding
LDFLAGS = -Wl,--relax

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

$(PROG).elf: $(PROG).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(PROG).hex: $(PROG).elf
	$(OBJCOPY) -j .text -j .data -O ihex $^ $@

$(PROG)_eeprom.hex: $(PROG).elf
	$(OBJCOPY) -j .eeprom -O ihex $^ $@

clean::
	rm -f *~ $(PROG).hex $(PROG)_eeprom.hex $(PROG).elf $(PROG).o

upload: $(PROG).hex
	avrdude -c usbasp -p m8 -u -U flash:w:$(PROG).hex

upload_eeprom: $(PROG)_eeprom.hex
	avrdude -c usbasp -p m8 -u -U eeprom:w:$(PROG)_eeprom.hex
