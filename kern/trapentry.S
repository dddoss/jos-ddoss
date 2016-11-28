/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps;                                                  \
        .data;                                                          \
            .long name;                                                 \
        .text

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps;                                                  \
        .data;                                                          \
            .long name;                                                 \
        .text

.data
    .globl vectors;
    vectors:
.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(vector0, T_DIVIDE);
TRAPHANDLER_NOEC(vector1, T_DEBUG);
TRAPHANDLER_NOEC(vector2, T_NMI); /* Reserved */
TRAPHANDLER_NOEC(vector3, T_BRKPT);
TRAPHANDLER_NOEC(vector4, T_OFLOW);
TRAPHANDLER_NOEC(vector5, T_BOUND);
TRAPHANDLER_NOEC(vector6, T_ILLOP);
TRAPHANDLER_NOEC(vector7, T_DEVICE);
TRAPHANDLER(vector8, T_DBLFLT);
TRAPHANDLER_NOEC(vector9, T_COPROC); /* Reserved */
TRAPHANDLER(vector10, T_TSS);
TRAPHANDLER(vector11, T_SEGNP);
TRAPHANDLER(vector12, T_STACK);
TRAPHANDLER(vector13, T_GPFLT);
TRAPHANDLER(vector14, T_PGFLT);
TRAPHANDLER_NOEC(vector15, T_RES); /* Reserved */
TRAPHANDLER_NOEC(vector16, T_FPERR);
TRAPHANDLER(vector17, T_ALIGN);
TRAPHANDLER_NOEC(vector18, T_MCHK);
TRAPHANDLER_NOEC(vector19, T_SIMDERR);
TRAPHANDLER_NOEC(vector20, 20);
TRAPHANDLER_NOEC(vector21, 21);
TRAPHANDLER_NOEC(vector22, 22);
TRAPHANDLER_NOEC(vector23, 23);
TRAPHANDLER_NOEC(vector24, 24);
TRAPHANDLER_NOEC(vector25, 25);
TRAPHANDLER_NOEC(vector26, 26);
TRAPHANDLER_NOEC(vector27, 27);
TRAPHANDLER_NOEC(vector28, 28);
TRAPHANDLER_NOEC(vector29, 29);
TRAPHANDLER_NOEC(vector30, 30);
TRAPHANDLER_NOEC(vector31, 31);
TRAPHANDLER_NOEC(vector32, IRQ_OFFSET);
TRAPHANDLER_NOEC(vector33, IRQ_OFFSET+1);
TRAPHANDLER_NOEC(vector34, IRQ_OFFSET+2);
TRAPHANDLER_NOEC(vector35, IRQ_OFFSET+3);
TRAPHANDLER_NOEC(vector36, IRQ_OFFSET+4);
TRAPHANDLER_NOEC(vector37, IRQ_OFFSET+5);
TRAPHANDLER_NOEC(vector38, IRQ_OFFSET+6);
TRAPHANDLER_NOEC(vector39, IRQ_OFFSET+7);
TRAPHANDLER_NOEC(vector40, IRQ_OFFSET+8);
TRAPHANDLER_NOEC(vector41, IRQ_OFFSET+9);
TRAPHANDLER_NOEC(vector42, IRQ_OFFSET+10);
TRAPHANDLER_NOEC(vector43, IRQ_OFFSET+11);
TRAPHANDLER_NOEC(vector44, IRQ_OFFSET+12);
TRAPHANDLER_NOEC(vector45, IRQ_OFFSET+13);
TRAPHANDLER_NOEC(vector46, IRQ_OFFSET+14);
TRAPHANDLER_NOEC(vector47, IRQ_OFFSET+15);
TRAPHANDLER_NOEC(vector48, T_SYSCALL);

/*
 * Lab 3: Your code here for _alltraps
 */

_alltraps:
    pushl %ds;
    pushl %es;
    pushal;
    movw $(GD_KD), %ax
    movw %ax, %ds;
    movw %ax, %es;
    pushl %esp;
    call trap;
    addl $4, %esp

.globl trapret
trapret:
  popal
  popl %es
  popl %ds
  addl $0x8, %esp /* trapno and err */
  iret
