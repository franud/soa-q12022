#include <asm.h>

/*
 * wrappers.S - System Call wrappers.
 */

ENTRY(write)
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx
	pushl %ecx
	pushl %edx

	movl 0x8(%ebp), %ebx
	movl 0xc(%ebp), %ecx
	movl 0x10(%ebp), %edx

	movl $4, %eax
	
	pushl $1f

	pushl %ebp
	mov %esp, %ebp

	//int $0x80 
	sysenter
1:
	popl %ebp
	addl $4, %esp

	popl %edx
	popl %ecx
	popl %ebx

	cmpl $0, %eax
	jge 2f

	negl %eax
	movl %eax, errno
	movl $-1, %eax
2:
	popl %ebp
	ret

ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp

	movl $20, %eax

	pushl %ecx
	pushl %edx

	pushl $1f

	pushl %ebp
	mov %esp, %ebp

	sysenter
1:
	pop %ebp
	addl $4, %esp

	popl %edx
	popl %ecx

	cmpl $0, %eax
	jge 2f

	negl %eax
	movl %eax, errno
	movl $-1, %eax
2:
	popl %ebp
	ret



ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx
	pushl %ecx
	pushl %edx

	movl $10, %eax
	
	pushl $1f

	pushl %ebp
	mov %esp, %ebp

	//int $0x80 
	sysenter
1:
	popl %ebp
	addl $4, %esp

	popl %edx
	popl %ecx
	popl %ebx

	cmpl $0, %eax
	jge 2f

	negl %eax
	movl %eax, errno
	movl $-1, %eax
2:
	popl %ebp
	ret
