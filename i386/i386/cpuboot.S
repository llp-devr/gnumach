//#include <mach/machine/asm.h>
//#include <i386/i386asm.h>
//#include <i386/vm_param.h> //phystokv

    .section .boot, "awx"

    .align 0x10

    .text
    .globl apstart, _apstart, counter

apstart:
_apstart:
	jmp boot_start

	#define MULTIBOOT_MAGIC 0x1badb002
	#define MULTIBOOT_FLAGS 0x0
	#define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)
	#define STACK_SIZE (4096 * 2)

ap_data:
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM

boot_start:
    /* loading idt and gdt */
    lidt idt_descr-0xc0000000
    lgdt gdt_descr-0xc0000000

    ljmp $0x8, $1f
0:
    mov $0x10, %eax
    movw %ax, %ds
    movw %ax, %es

    movw %ax, %ss
    movl $(stack_bsp + STACK_SIZE), %esp


    /* first argument: multiboot */
    push %ebx

    cli
0:
    hlt
    jmp 1b

    .section .rodata

    .align 0x10

    .globl idt_descr
idt_descr:
    .word 0
    .long 0

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

    .bss
    .p2align 4
    .globl stack_bsp
    .comm stack_bsp, STACK_SIZE


#define M(addr) (addr - apboot + 0x7000)

    .text
    .globl apboot, apbootend, stack_ptr
    .align 0x10
    .code16
apboot:
_apboot:


    cli
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x7000, %sp

    lgdt M(gdt_descr_tmp)

    mov %cr0, %eax
    or $0x1, %eax
    mov %eax, %cr0

    ljmp $0x8, $M(1f)


counter:
.long 0

0:
	.code32

	mov $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $0x7000, %esp

	incw M(counter)

	lidtl idt_descr-KERNELBASE

	incw M(counter)

//    lgdt boot_gdt_descr-KERNELBASE
//    ljmpl $0x8, $(1f-KERNELBASE)

//1:
//    mov $0x10, %eax
//    movw %ax, %ds
//    movw %ax, %es


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
	movl	$_intstack+INTSTACK_SIZE,%esp

	/* Reset EFLAGS to a known state.  */
	pushl	$0
	popf



    movw %ax, %ss
    movl stack_ptr, %esp
    addl $STACK_SIZE, %esp

    incw M(counter)

    pushl stack_ptr

    incw M(counter)

    call cpu_ap_main-KERNELBASE


    cli
0:
    hlt
    jmp 1b

stack_ptr:
    .long 0x0

    .align 0x10
gdt_descr_tmp:
    .short 0x8*3 - 1
    .long M(gdt_tmp)

    .align 0x10

gdt_tmp:
    /* 0 */
    .quad 0
    /* boot CS = 0x8 */
    .word 0xffff
    .word 0x0000
    .byte 0x00
    .byte 0x9a
    .byte 0xcf
    .byte 0x00
    /* boot DS = 0x10 */
    .word 0xffff
    .word 0x0000
    .byte 0x00
    .byte 0x92
    .byte 0xcf
    .byte 0x00


_apbootend:
apbootend:

    .globl dummyf
dummyf:
ret
