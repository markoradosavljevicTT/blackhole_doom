OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
/*OUTPUT_ARCH(rv32i)*/

ENTRY(_start)

SECTIONS
{
    . = 0x00000000;
    .init :
    {
	*(.init)
    }
    .text :
    {
	*(.text)
	*(.text.*)
    *(.critical)
    }
    ASSERT(. <= 0x1EF00, "text overflowed into sim page / framebuffer 1")

    . = 0x0003E500;
    .data :
    {
	*(.data)
	*(.data.*)
	*(.sdata)
	*(.sdata.*)
    }

    .bss :
    {
	*(.bss)
	*(.bss.*)
	*(.sbss)
	*(.sbss.*)
    }

    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
        *(.srodata)
        *(.srodata.*)
    }
    ASSERT(. <= 0x3FC000, "rodata overflowed into stack")

    . = 0x14000000;

    /DISCARD/ :
    {
        *(.debug_info)
        *(.debug_abbrev)
        *(.debug_aranges)
        *(.debug_line)
        *(.debug_str)
        *(.debug_line_str)
        *(.debug_frame)
        *(.comment)
        *(.riscv.attributes)
        *(.symtab)
        *(.strtab)
        *(.shstrtab)
    }
}
