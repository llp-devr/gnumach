
#include <mach/machine/asm.h>
#include <i386/apic.h>
#include <i386/i386asm.h>

	/*
	 * This section will be put first into .text.  See also	i386/ldscript.
	 */
	.section .text.start,"ax"
	
	/* We should never be entered this way.  */
	.globl	start,_start
start:
_start:
	jmp	boot_entry

	/* MultiBoot header - see multiboot.h.  */
#define	MULTIBOOT_MAGIC		0x1BADB002
#ifdef __ELF__
#define MULTIBOOT_FLAGS		0x00000003
#else  /* __ELF__ */
#define MULTIBOOT_FLAGS		0x00010003
#endif /* __ELF__ */
	P2ALIGN(2)
boot_hdr:
	.long	MULTIBOOT_MAGIC
	.long	MULTIBOOT_FLAGS
		/*
		 * The next item here is the checksum.
		 * XX this works OK until we need at least the 30th bit.
		 */
	.long	- (MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)
#ifndef __ELF__	/* a.out kludge */
	.long	boot_hdr	/* header_addr */
	.long	_start		/* load_addr */
	.long	_edata		/* load_end_addr */
	.long	_end		/* bss_end_addr */
	.long	boot_entry	/* entry */
#endif /* __ELF__ */

boot_entry:
	/* use segmentation to offset ourself. */
	lgdt	boot_gdt_descr - KERNELBASE
	ljmp	$8,$0f
0:
	movw	$0,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%fs
	movw	%ax,%gs
	movw	$16,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%ss

	/* Switch to our own interrupt stack.  */
	movl	$solid_intstack+INTSTACK_SIZE-4, %esp
	andl	$0xfffffff0,%esp

	/* Enable local apic */
	xorl	%eax, %eax
	xorl	%edx, %edx
	movl	$APIC_MSR, %ecx
	rdmsr
	orl	$APIC_MSR_ENABLE, %eax
	orl	$APIC_MSR_BSP, %eax
	movl	$APIC_MSR, %ecx
	wrmsr

	/* Reset EFLAGS to a known state.  */
	pushl	$0
	popf

	/* Clear uninitialized data.  */
	lea     _edata,%edi
	lea     _end,%ecx
	subl    %edi,%ecx
	xorl    %eax,%eax
	rep
	stosb

	/* Push the boot_info pointer to be the second argument.  */
	pushl	%ebx

	/* Fix ifunc entries */
	movl    $__rel_iplt_start,%esi
	movl    $__rel_iplt_end,%edi
iplt_cont:
	cmpl    %edi,%esi
	jae     iplt_done
	movl    (%esi),%ebx	/* r_offset */
	movb    4(%esi),%al	/* info */
	cmpb    $42,%al		/* IRELATIVE */
	jnz     iplt_next
	call    *(%ebx)		/* call ifunc */
	movl    %eax,(%ebx)	/* fixed address */
iplt_next:
	addl    $8,%esi
	jmp     iplt_cont
iplt_done:

	/* Jump into C code.  */
	call	EXT(c_boot_entry)

.align 16
	.word 0
boot_gdt_descr:
	.word 3*8+7
	.long boot_gdt - KERNELBASE
.align 16
boot_gdt:
	/* 0 */
	.quad 0
	/* boot CS = 8 */
	.word 0xffff
	.word (-KERNELBASE) & 0xffff
	.byte ((-KERNELBASE) >> 16) & 0xff
	.byte 0x9a
	.byte 0xcf
	.byte ((-KERNELBASE) >> 24) & 0xff
	/* boot DS = 8 */
	.word 0xffff
	.word (-KERNELBASE) & 0xffff
	.byte ((-KERNELBASE) >> 16) & 0xff
	.byte 0x92
	.byte 0xcf
	.byte ((-KERNELBASE) >> 24) & 0xff

