/*
 * include/asm-i386/processor.h
 *
 * Copyright (C) 1994 Linus Torvalds
 */

#ifndef __ASM_I386_PROCESSOR_H
#define __ASM_I386_PROCESSOR_H

#include <asm/vm86.h>
#include <asm/math_emu.h>

/*
 * System setup and hardware bug flags..
 * [Note we don't test the 386 multiply bug or popad bug]
 */

extern char hard_math;
extern char x86;		/* lower 4 bits */
extern char x86_vendor_id[13];
extern char x86_model;		/* lower 4 bits */
extern char x86_mask;		/* lower 4 bits */
extern int  x86_capability;	/* field of flags */
extern int  fdiv_bug;		
extern char ignore_irq13;
extern char wp_works_ok;	/* doesn't work on a 386 */
extern char hlt_works_ok;	/* problems on some 486Dx4's and old 386's */
extern int  have_cpuid;		/* We have a CPUID */

extern unsigned long cpu_hz;	/* CPU clock frequency from time.c */

#if 0
/*
 *	Detection of CPU model (CPUID).
 */
extern inline void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
	__asm__("cpuid"
		: "=a" (*eax),
		  "=b" (*ebx),
		  "=c" (*ecx),
		  "=d" (*edx)
		: "a" (op)
		: "cc");
}
#endif

/*
 * Cyrix CPU register indexes (use special macros to access these)
 */
#define CX86_CCR2 0xc2
#define CX86_CCR3 0xc3
#define CX86_CCR4 0xe8
#define CX86_CCR5 0xe9
#define CX86_DIR0 0xfe
#define CX86_DIR1 0xff

/*
 * Cyrix CPU register access macros
 */

extern inline unsigned char getCx86(unsigned char reg)
{
	unsigned char data;

	__asm__ __volatile__("movb %1,%%al\n\t"
		      "outb %%al,$0x22\n\t"
		      "inb $0x23,%%al" : "=a" (data) : "q" (reg));
	return data;
}

extern inline void setCx86(unsigned char reg, unsigned char data)
{
	__asm__ __volatile__("outb %%al,$0x22\n\t"
	     "movb %1,%%al\n\t"
	     "outb %%al,$0x23" : : "a" (reg), "q" (data));
}

/*
 * Bus types (default is ISA, but people can check others with these..)
 * MCA_bus hardcoded to 0 for now.
 */
extern int EISA_bus;
#define MCA_bus 0
#define MCA_bus__is_a_macro /* for versions in ksyms.c */

/*
 * User space process size: 3GB. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */
#define TASK_SIZE	(0xC0000000UL)
#define MAX_USER_ADDR	TASK_SIZE
#define MMAP_SEARCH_START (TASK_SIZE/3)

/*
 * Size of io_bitmap in longwords: 32 is ports 0-0x3ff.
 */
#define IO_BITMAP_SIZE	32

struct i387_hard_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
	long	status;		/* software status information */
};

struct i387_soft_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long    top;
	struct fpu_reg	regs[8];	/* 8*16 bytes for each FP-reg = 128 bytes */
	unsigned char	lookahead;
	struct info	*info;
	unsigned long	entry_eip;
};

union i387_union {
	struct i387_hard_struct hard;
	struct i387_soft_struct soft;
};

struct thread_struct {
	unsigned short	back_link,__blh;
	unsigned long	esp0;
	unsigned short	ss0,__ss0h;
	unsigned long	esp1;
	unsigned short	ss1,__ss1h;
	unsigned long	esp2;
	unsigned short	ss2,__ss2h;
	unsigned long	cr3;
	unsigned long	eip;
	unsigned long	eflags;
	unsigned long	eax,ecx,edx,ebx;
	unsigned long	esp;
	unsigned long	ebp;
	unsigned long	esi;
	unsigned long	edi;
	unsigned short	es, __esh;
	unsigned short	cs, __csh;
	unsigned short	ss, __ssh;
	unsigned short	ds, __dsh;
	unsigned short	fs, __fsh;
	unsigned short	gs, __gsh;
	unsigned short	ldt, __ldth;
	unsigned short	trace, bitmap;
	unsigned long	io_bitmap[IO_BITMAP_SIZE+1];
	unsigned long	tr;
	unsigned long	cr2, trap_no, error_code;
/* floating point info */
	union i387_union i387;
/* virtual 86 mode info */
	struct vm86_struct * vm86_info;
	unsigned long screen_bitmap;
	unsigned long v86flags, v86mask, v86mode;
};

#define INIT_MMAP { &init_mm, 0, 0x40000000, PAGE_SHARED, VM_READ | VM_WRITE | VM_EXEC }

#define INIT_TSS  { \
	0,0, \
	sizeof(init_kernel_stack) + (long) &init_kernel_stack, \
	KERNEL_DS, 0, \
	0,0,0,0,0,0, \
	(long) &swapper_pg_dir, \
	0,0,0,0,0,0,0,0,0,0, \
	USER_DS,0,USER_DS,0,USER_DS,0,USER_DS,0,USER_DS,0,USER_DS,0, \
	_LDT(0),0, \
	0, 0x8000, \
	{~0, }, /* ioperm */ \
	_TSS(0), 0, 0,0, \
	{ { 0, }, },  /* 387 state */ \
	NULL, 0, 0, 0, 0 /* vm86_info */ \
}

#define alloc_kernel_stack()    __get_free_page(GFP_KERNEL)
#define free_kernel_stack(page) free_page((page))

static inline void start_thread(struct pt_regs * regs, unsigned long eip, unsigned long esp)
{
	regs->cs = USER_CS;
	regs->ds = regs->es = regs->ss = regs->fs = regs->gs = USER_DS;
	regs->eip = eip;
	regs->esp = esp;
}

/*
 * Return saved PC of a blocked thread.
 */
extern inline unsigned long thread_saved_pc(struct thread_struct *t)
{
	return ((unsigned long *)t->esp)[3];
}

#endif /* __ASM_I386_PROCESSOR_H */
