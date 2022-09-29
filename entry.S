/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>


/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es;

#define RESTORE_ALL \
      popl %ebx; \
      popl %ecx; \
      popl %edx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs; 

#define EOI \
      movb $0x20, %al; \
      outb %al, $0x20 ;

// clock_handler
/* ENTRY(clock_handler)
        SAVE_ALL
        call clock_routine
        RESTORE_ALL
        iret
// */


// System call handler
ENTRY(system_call_handler)
	SAVE_ALL				// Save context
	cmpl $0, %eax				// is syscall number negative?
	jl err					// if sc_number < 0, return error.
	cmpl $MAX_SYSCALL, %eax			// check if syscall number is higher than the syscall_table's range
	jg err					// return error if it is.
	call *sys_call_table(, %eax, 0x04)	// sys_call_table address + %eax * 4 => call the corresponding routine (table entry specified by eax register).
	jmp fin					// Finish routine
err:
	movl $-ENOSYS, %eax			// Return ENOSYS error.
fin:
	movl %eax, 0x18(%esp)			// Store, in the stack, the %eax value returned by the routine so it isn't smashed by RESTORE_ALL.
	RESTORE_ALL				// restore context
	iret


// keyboard_handler
ENTRY(keyboard_handler)
      SAVE_ALL
      call keyboard_routine
      RESTORE_ALL
      EOI
      iret;
