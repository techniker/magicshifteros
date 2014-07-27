# MagicShifterOS #

Everything you need to modify and customize your MagicShifter

The most important directories are:

### /Case ###
All source (OpenSCAD + dxf) and STL files for the case and the first MagicShifter accessory: a bike mount.

### /Electronics ###
The original EAGLE design files of the MagicShifter PRO board (CC BY-NC-SA 3.0 http://creativecommons.org/licenses/by-nc-sa/3.0/), a PDF of the scematics and the BOM (Bill of Material) that documents every single resistor, capacitor, connector and IC I used in the design. I chose the CC BY-NC-SA to prevent crappy rip off copies. We use very high quality components from Osram, Cree, Texas Instruments, Freescale, austriamicrosystems, etc. and I fear in cheap MagicShifter clones these will be replaced by inferior replacements. If you make a true derivative of the design I will happily grant almost any exception to the license (free of charge of course) since I was hoping to spark a new wave of small, smart, embedded ultra low power systems for gaming, fashion & lighting with the release of my MagicShifter design :) 

I tried to give full credit to all the open source designs that influenced the MagicShifter design. If I forgot to reference any project please send me an email so that I may correct that mistake.

### /Firmware ###
Here is the code and the Makefile (generously written by Clifford Wolf http://www.clifford.at/). 

To build the firmware you have to copy the unzipped arduino software into a (not version controlled) "arduino" subfolder in the "Firmware" folder to compile the firmware yourself. Arduino is required since we use the Arduino USB implementation. 

If you don't want to build the firmware yourself you can use the precompiled HEX file in the "precompiled" subdirectory.

### /Software 
in the python subfolder you can find the tools for uploading custom images and custom firmware

* * *

please report any bugs you find in the ticket system or just send me a mail to:

wizards23+magicshifter@gmail.com

* * *

greetings from the future!

Philipp aka wizard23 aka babygiraffe

:: open source hardware/magic

:: http://magicshifter.net/

:: http://metalab.at/


Any sufficiently advanced technology is indistinguishable from a MagicShifter.

--Arthur C. Clarke