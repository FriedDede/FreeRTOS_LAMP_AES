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

#ifdef QEMU
mem_lenght		= 512K;
mem_base 		= 0x80000000;
vector_base 	= 0x00000000;
vector_lenght  	= 0x0;

#elif LAMP
/* LAMP
*/
multiplier 		= CPU_ID;
mem_lenght		= 512K;
mem_base 		= 0x00000100 	+ (multiplier*mem_lenght);
vector_base 	= 0x00000000 	+ (multiplier*mem_lenght);
vector_lenght  	= 0x100;
#endif

MEMORY
{
    /* Memory section to accommodate the vector table */
    VECTORS (rwx) : ORIGIN = vector_base, LENGTH = vector_lenght
    /* Memory section to store program and data */
    mem (rwx) : ORIGIN = mem_base, LENGTH = mem_lenght
}

__min_stack      = 0x800;   /* 2K minimum stack */

SECTIONS
{
	.init :
	{
		_text = .;
		KEEP (*(SORT_NONE(.init)))
	} >mem AT>mem
	
	.text :
	{
        . = ALIGN(4);
		*(.text.unlikely .text.unlikely.*)
		*(.text.startup .text.startup.*)
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
        . = ALIGN(4);
	} >mem AT>mem

	.fini :
	{
		KEEP (*(SORT_NONE(.fini)))
		_etext = .;
	} >mem AT>mem

	.rodata.align :
	{
		. = ALIGN(4);
		_rodata = .;
	} >mem AT>mem

	.rodata.start :
	{
		_rodata_lma = LOADADDR(.rodata.start);
	} >mem AT>mem

	.rodata :
	{
		*(.rdata)
		*(.rodata .rodata.*)
		*(.gnu.linkonce.r.*)

		. = ALIGN(4);
		_erodata = .;
	} >mem AT>mem

	.data.align :
	{
		. = ALIGN(4);
		_data = .;
	} >mem AT>mem

	.data.start :
	{
		_data_lma = LOADADDR(.data.start);
	} >mem AT>mem

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
	} >mem AT>mem

	.bss.align :
	{
		. = ALIGN(4);
		_bss = .;
	} >mem AT>mem

	.bss.start :
	{
		_bss_lma = LOADADDR(.bss.start);
	} >mem AT>mem

	.bss :
	{
		*(.sbss*)
		*(.gnu.linkonce.sb.*)
		*(.bss .bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;
	} >mem AT>mem

	. = ALIGN(8);
	_end = .;

	.stack :
	{
		. = ALIGN(16);
		. += __stack_size;
		_stack_top = .;
	} >mem AT>mem
}

