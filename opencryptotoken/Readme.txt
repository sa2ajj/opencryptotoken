This is the README file for the OpenCryptoToken (Based on PowerSwitch
example project from Objective Developement)

BUILDING AND INSTALLING

Both, the firmware and Unix command line tool are built with "make". You may
need to customize both makefiles.

Firmware:
The firmware for OpenCryptoToken requires avr-gcc and avr-libc (a C-library for
the AVR controller). Please read the instructions at
http://www.nongnu.org/avr-libc/user-manual/install_tools.html for how to
install the GNU toolchain (avr-gcc, assembler, linker etc.) and avr-libc.
If you are on Windows, download WinAVR (see http://winavr.sourceforge.net/).

Once you have the GNU toolchain for AVR microcontrollers installed, you can
run "make" in the subdirectory "firmware". You may have to edit the Makefile
to use your preferred downloader with "make flash". Our current version is
built for uisp with the AVR910 serial programmer connected through a USB to
serial adapter to a Mac running Mac OS X.

Command Line Tool on Unix:
The command line tool requires libusb. Please download libusb from
http://libusb.sourceforge.net/ and install it before you compile. Change to
directory "commandline", check the Makefile and edit the settings if required
and type "make". This will build the unix executable "powerSwitch" which can
be used to control the device.

Command Line Tool on Windows:
The command line tool uses libusb-win32. Please download libusb-win32 from
http://libusb-win32.sourceforge.net/. The small package
(libusb-win32-device-bin) should be sufficient. We need usb.h and libusb.a for
gcc (or your favorite compiler). Make sure that usb.h is in the include path
and libusb.a in the library search path (edit Makefile to achieve this).
Although it should be possible to compile the command line tool with any
compiler, we recommend that you use MSYS (to run make and other Unix tools)
and MinGW (for the gcc toolchain). Both can be downloaded for free from
http://www.mingw.org/.

If you want to develop Windows drivers without libusb-win32, see our
"Automator" project for an example.


WORKING WITH POWERSWITCH

Once you have compiled and flashed the firmware and compiled the command line
tool, you can move on to the fun part: Connect the device to a free USB port
of your computer or USB-hub. You should now be able to find the device in
any USB analyzer tool, e.g. SystemProfiler on Mac OS X. If you connect it
to a Windows PC, the "New Hardware" dialog will pop up and you can create
the appropriate info files with libusb-win32's inf-wizard.

The next step is to try the command line tool, e.g.:

    ./powerSwitch off 3

to turn off port 3 (all ports are on when the device has been cleared) or

    for i in 0 1 2 3 4 5 6 7; do ./powerSwitch off $i; done

in an sh or bash (not csh or tcsh) type shell to turn off all ports.

You can also switch a port on or off temporarily. Try

    ./powerSwitch on 0 2.5

to activate port 0 for two and a half seconds.


FILES IN THE DISTRIBUTION

Readme.txt ........ The file you are currently reading.
firmware .......... Source code of the controller firmware.
firmware/usbdrv ... USB driver -- See Readme.txt in this directory for info
commandline ....... Source code of the host software (needs libusb).
circuit ........... Circuit diagrams in PDF and EAGLE 4 format. A free version
                    of EAGLE is available for Linux, Mac OS X and Windows from
                    http://www.cadsoft.de/.
License.txt ....... Public license for all contents of this project.
Changelog.txt ..... Logfile documenting changes in soft-, firm- and hardware.
Pitfalls.txt ...... List of possible pitfalls and solutions.


USING THE USB DRIVER FOR YOUR OWN PROJECTS

OpenCryptoToken is mainly an example, although it may be very useful by itself.
If you want to implement your own application based on our USB driver, see
the file "firmware/main.c" for the application specific part of the firmware.
OpenCryptoToken uses only the most basic driver interface. More information and a
full technical description of the driver's interface can be found in
"firmware/usbdrv/usbdrv.h". Please review the configuration options in
"firmware/usbconfig.h". OpenCryptoToken uses Objective Development's free shared
USB IDs by default. You may set your own USB IDs in usbconfig.h. For more
information about USB descriptors and IDs see the document "USB in a Nutshell"
available at http://www.beyondlogic.org/usbnutshell/usb1.htm or the USB 1.1
standard available at http://www.usb.org/. Don't forget to check the license
agreement for Objective Development's firmware-only USB driver!


ABOUT THE LICENSE

It is our intention to make our USB driver and this demo application
available to everyone. Moreover, we want to make a broad range of USB
devices and ideas for USB devices available to the general public. We
therefore require that all projects built with our USB driver are published
under an Open Source license. Our license for the USB driver and demo code
is based on the GNU General Public License (GPL). We add some special
requirements and privileges, though. See the file "License.txt" for details.

If you don't want to publish your source code and the circuit diagrams under
an Open Source license, you can simply pay money for AVR-USB. As an
additional benefit you get USB PIDs for free, licensed exclusively to you.
See http://www.obdev.at/products/avrusb/license.html for details.


MORE INFORMATION

For more information about Objective Development's firmware-only USB driver
for Atmel's AVR microcontrollers please visit the URL

    http://www.obdev.at/products/avrusb/

A technical documentation of the driver's interface can be found in the file
"firmware/usbdrv/usbdrv.h".



(c) 2005, 2006 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
