# Name: Makefile
# Project: ectl, based on powerSwitch
# Author: mrk
# Creation Date: 2006-09-16
# Copyright: (c) 2006 by ITS
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile 97 2006-01-26 18:38:51Z cs $


# Concigure the following definitions according to your system.

CC              = gcc
LIBUSB_CONFIG   = libusb-config
# Make sure that libusb-config is in the search path or specify a full path.
# On Windows, there is no libusb-config and you must configure the options
# below manually. See examples.
CFLAGS          = `$(LIBUSB_CONFIG) --cflags` -O -Wall
#CFLAGS          = -I/usr/local/libusb/include
# On Windows replace `$(LIBUSB_CONFIG) --cflags` with appropriate "-I..."
# option to ensure that usb.h is found
LIBS            = `$(LIBUSB_CONFIG) --libs`
#LIBS            = `$(LIBUSB_CONFIG) --libs` -framework CoreFoundation
# You may need "-framework CoreFoundation" on Mac OS X and Darwin.
#LIBS            = -L/usr/local/libusb/lib/gcc -lusb
# On Windows use somthing similar to the line above.

all: ectl

.c.o:
	$(CC) $(CFLAGS) -c $<

ectl: ectl.o
	$(CC) -o ectl ectl.o $(LIBS)

clean:
	rm -f *.o
	rm -f ectl ectl.exe
