/* The apploader runs at 0x81200000 (see apploader.c). Everything, the BSS
   included, goes in one section so that objcopy -O binary writes a single
   image with the zeroed data in it. */

OUTPUT_ARCH(powerpc:common)
ENTRY(apploader_entry)

SECTIONS
{
  . = 0x81200000;
  .apploader : {
    *(.text.entry)
    *(.text .text.*)
    *(.rodata .rodata.*)
    *(.data .data.*)
    *(.sdata .sdata.*)
    *(.sdata2 .sdata2.*)
    *(.sbss .sbss.*)
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(32);
  }
  /DISCARD/ : {
    *(.comment)
    *(.eh_frame)
    *(.gnu.attributes)
  }
}
