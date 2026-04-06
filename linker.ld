ENTRY(start)

SECTIONS
{
    . = 1M;

    .boot :
    {
        KEEP(*(.multiboot_header))
    }

    .text :
    {
        *(.text)
    }

    /* Heap */
    . = ALIGN(0x1000);
    __heap_start = .;

    .heap (NOLOAD) :
    {
        . = . + 0x00100000; /* 1 MB heap */
    }
    __heap_end = .;
    . = ALIGN(0x1000);

    /* Stack */
    . = ALIGN(0x1000);
    __stack_top = .;

    .stack (NOLOAD) :
    {
        . = . + 0x00010000; /* 64 KB stack */
    }
    __stack_bottom = .;
    . = ALIGN(0x1000);
}
