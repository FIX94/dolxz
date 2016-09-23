/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_start)

PHDRS {
  app PT_LOAD FLAGS(5);
  bss PT_LOAD FLAGS(6);
  dat PT_LOAD FLAGS(6);
  dat2 PT_LOAD FLAGS(4);
}

SECTIONS {
	. = 0x80003400;

	.start : { *(.init) *(.init.*) } :app
	.text : { *(.text) *(.text.*) } :app

	__bss_start = .;
	.bss : { *(.bss) *(.bss.*) } :bss
	__bss_end = .;

	.data : { *(.data) *(.data.*) } :dat
	.sdata : { *(.sdata) *(.sdata.*) } :dat
	.rodata : { *(.rodata) *(.rodata.*) } :dat

	. = 0x80040000;
	.rodata1 : { *(.rodata1) } :dat2
}

