# Name: Makefile
# Project: PowerSwitch
# Author: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile 147 2006-03-01 17:33:03Z cs $

LDFLAGS += -Wl,--script=avr4.x -Wl,-Tdata,0x8000a0


#SERIAL = `echo /dev/tty.KeySerial*`
#UISP = uisp -dprog=avr910 -dserial=$(SERIAL) -dpart=auto
#UISP = uisp -dprog=dapa
UISP=uisp -dprog=dapa -dlpt=/dev/.static/dev/parport0
# The two lines above are for "uisp" and the AVR910 serial programmer connected
# to a Keyspan USB to serial converter to a Mac running Mac OS X.
# Choose your favorite programmer and interface.

#COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=at90s2313 #-DDEBUG_LEVEL=2
COMPILE = avr-gcc -Wall -Os -I../../avrusbboot/firmware/usbdrv/ -I. -mmcu=atmega8 -g #-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.

OBJECTS = main.o ecasm.o ec.o appvect.o privkey.o file.o
# Note that we link usbdrv.o first! This is required for correct alignment of
# driver-internal global variables!


# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(UISP) --erase --upload --verify if=main.hex

usbflash:	all
	../../avrusbboot/software/avrusbboot main.hex

# Fuse low byte:
# 0xef = 1 1 1 0   1 1 1 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> crystal @ 12 MHz)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
#
# Fuse high byte:
# 0xdb = 1 1 0 1   1 0 1 1
#        ^ ^ ^ ^   \-+-/ ^
#        | | | |     |   +---- RSTDISBL (disable external reset -> enabled)
#        | | | |     +-------- BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | +-------------- WDTON (watchdog timer always on -> disable)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ EESAVE (preserve EEPROM on Chip Erase -> not preserved)
#        +-------------------- DWEN (debug wire enable)
fuse_tiny2313:	# only needed for attiny2313
	$(UISP) --wr_fuse_l=0xef --wr_fuse_h=0xdb


clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.bin *.o usbdrv/*.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
main.bin:	$(OBJECTS)
	$(COMPILE) -o main.bin $(OBJECTS) $(LDFLAGS)

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex
	./checksize main.bin
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(COMPILE) -E main.c

cert.pem: ecc-secp192r1-client.pfx
	openssl pkcs12 -in ecc-secp192r1-client.pfx -out cert.pem -passin pass:password -passout pass:password

cert.der: cert.pem
	openssl x509 -in cert.pem -outform DER -out cert.der

privkey.c: cert.pem
	echo "#include <avr/pgmspace.h>">privkey.c
	result="";for i in `openssl ec -passin pass:password -in cert.pem -text|grep priv -A 2 |grep "^ " |tr -d ' \n' |tr ':' ' '`; do result="0x$$i,$$result"; done; echo "unsigned char P_private_key[] PROGMEM = { $$result };">>privkey.c

cert.h: cert.pem
	tab=`openssl x509 -in cert.pem -C|grep _certificate -A 1000|grep "^0x"`;echo "unsigned char P_certificate[] PROGMEM = { $$tab };" >>cert.h

main.o: cert.h main.c
