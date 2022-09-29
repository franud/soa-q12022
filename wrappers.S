#include <asm.h>

/*
 * wrappers.S - System Call wrappers.
 */

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx
	pushl %edx
	pushl %ecx

	movl 0x8(%ebp), %ebx
	movl 0xc(%ebp), %ecx
	movl 0x10(%ebp), %edx

	movl $4, %eax
	
	pushl $1f
	pushl %ebp
	mov %esp, %ebp

	int $0x80

1:
	popl %ebp
	addl $4, %esp

	popl %ebx
	popl %ecx
	popl %edx

	cmpl $0, %eax
	jge 2f
	negl %eax
	movl %eax, errno
	movl -1, %eax
2:
	popl %ebp
	ret
