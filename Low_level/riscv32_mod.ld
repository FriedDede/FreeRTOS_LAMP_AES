/* Simple linker script for the lamp RISC-V core implementation
 * 
 * SUPPORTS:
 * -> bare metal applications 
 * -> single stack pointer
 *
 * TODO
 * -> multiple stack pointers for U S and M modes
 * -> interrupt handlers
 * -> init CSR
 * -> init peripherals, e.g., UART (to chat with host pc).
 * -> slim the linker script to gain more control
 */


OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(_start)

/* QEMU
*/
ram_base 		= 0x80000000;
rom_base 		= 0x80080000;
vector_base 	= 0x00000000;
vector_lenght  	= 0x0;

/* LAMP
*/
/*ram_base 		= 0x00000100;
rom_base 		= 0x00080100;
vector_base 	= 0x00000000;
vector_lenght  	= 0x100;
*/
MEMORY
{
    /* Memory section to accommodate the vector table */
    VECTORS (rwx) : ORIGIN = vector_base,     LENGTH = vector_lenght
    /* Memory section to store program and data */
    rom (rxa) : ORIGIN = ram_base, LENGTH = 512K
	ram (wxa) : ORIGIN = rom_base, LENGTH = 512K
}

__min_stack      = 0x800;   /* 2K minimum stack */

SECTIONS
{
	.init :
	{
		_text = .;
		KEEP (*(SORT_NONE(.init)))
	} >rom AT>rom
	
	.text :
	{
        . = ALIGN(4);
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
        . = ALIGN(4);
	} >rom AT>rom

	.fini :
	{
		KEEP (*(SORT_NONE(.fini)))
		_etext = .;
	} >rom AT>rom

	.rodata.align :
	{
		. = ALIGN(4);
		_rodata = .;
	} >rom AT>rom

	.rodata.start :
	{
		_rodata_lma = LOADADDR(.rodata.start);
	} >rom AT>rom

	.rodata :
	{
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)

		. = ALIGN(4);
		_erodata = .;
	} >rom AT>rom

	.data.align :
	{
		. = ALIGN(4);
		_data = .;
	} >ram AT>rom

	.data.start :
	{
		_data_lma = LOADADDR(.data.start);
	} >ram AT>rom

	.data :
	{
		*(.data .data.*)
		*(.gnu.linkonce.d.*)
		. = ALIGN(8);
		PROVIDE( __global_pointer$ = . + 0x800 );
		*(.sdata .sdata.*)
		*(.sdata2 .sdata2.*)
		*(.gnu.linkonce.s.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata .srodata.*)

		. = ALIGN(4);
		_edata = .;
	} >ram AT>rom

	.bss.align :
	{
		. = ALIGN(4);
		_bss = .;
	} >ram AT>rom

	.bss.start :
	{
		_bss_lma = LOADADDR(.bss.start);
	} >ram AT>rom

	.bss :
	{
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;
	} >ram AT>rom

	. = ALIGN(8);
	_end = .;

	.stack :
	{
		. = ALIGN(16);
		. += __stack_size;
		_stack_top = .;
	} >ram AT>ram
}

