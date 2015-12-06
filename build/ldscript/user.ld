ENTRY(_start)

SECTIONS
{
  . = 0x00010000;

  .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata)
    *(.rodata.*)
  }

  . = ALIGN(CONSTANT(MAXPAGESIZE)) - ((CONSTANT(MAXPAGESIZE) - .) & (CONSTANT(MAXPAGESIZE) - 1));
  . = DATA_SEGMENT_ALIGN(CONSTANT(MAXPAGESIZE), CONSTANT(COMMONPAGESIZE));

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }
}
