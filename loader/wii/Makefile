# Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
# GameCube Port Copyright 2016 FIX94
# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file LICENSE or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt


# Configuration:

# What toolchain prefix should we use
CROSS ?= powerpc-eabi-

# Where are the tools (http://git.infradead.org/users/segher/wii.git)
TOOLS ?= C:\segher_wii_tools

ifeq ($(OS),Windows_NT)
	BIN2H = ../../bin2h/bin2h.exe
else
	BIN2H = ../../bin2h/bin2h
endif

# End of configuration.


# Set CC, LD, OBJCOPY based on CROSS, unless they are set already

ifeq ($(origin CC), default)
	CC := $(CROSS)gcc -m32
endif
ifeq ($(origin LD), default)
	LD := $(CROSS)ld
endif
OBJCOPY ?= $(CROSS)objcopy

# The compiler flags we need.

CFLAGS := -Wall -W -O2 -ffreestanding -mno-eabi -mno-sdata -mcpu=750

# Build with "V=1" to see the commands executed; be quiet otherwise.

ifeq ($(V),1)
	Q :=
else
	Q := @
	MAKEFLAGS += --no-print-directory
endif

all:
	@echo "  ASSEMBLE  start.o"
	$(Q)$(CC) $(CFLAGS) -c start.S -o start.o
	@echo "  COMPILE   dolloader.o"
	$(Q)$(CC) $(CFLAGS) -c dolloader.c -o dolloader.o
	@echo "  LINK      dolloader.elf"
	$(Q)$(LD) start.o dolloader.o -T dolloader.ld -o dolloader.elf
	@echo "  OBJCOPY   dolloader.bin"
	$(Q)$(OBJCOPY) -Obinary dolloader.elf dolloader.bin
	@echo "  OUTPUT    dolloader.h"
	$(Q)$(BIN2H) dolloader.bin

clean:
	-rm -f start.o dolloader.o dolloader.elf dolloader.bin dolloader.h
