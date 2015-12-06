ENTRY(vectors_start)

SECTIONS
{
  . = 0x60008000;

  .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata)
    *(.rodata.*)
  }

  . = ALIGN(0x8000);

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }

  . = ALIGN(0x8000);

  /* 8KB */
  . = . + 0x2000;
  . = ALIGN(8);
  svc_stack = .;

  /* 8KB */
  . = . + 0x2000;
  . = ALIGN(8);
  irq_stack = .;
}
