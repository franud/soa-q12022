#include <asm.h>
/*
1) Parameter passing: ZeOS implements the parameter passing from user mode to system
mode through the CPU registers, as occurs in Linux. The parameters of the stack must be
copied to the registers EBX, ECX, EDX, ESI, EDI. The correct order is the first parameter (on
the left) in EBX, the second parameter in ECX, etc. Note that this order is a hack to exploit
the SAVE_ALL macro and to create the stack structure from section 3.5.
2) Put the identifier of the system call in the EAX register (number 4 for write).
3) Generate the trap: int $0x80.
4) Process the result (see section 3.8.2).
5) Return


int write (int fd, char * buffer, int size);
            ebx     ecx             edx


            | edx |
            | ecx |
            | ebx |
          ->| ebp |+0
            | ret |+4
            | fd  |+8
            | buf |+12
            |size |+16
*/

ENTRY(write)
    pushl %ebp
    movl %esp, %ebp

    pushl %ebx


    movl 8(%ebp), %ebx
    movl 12(%ebp), %ecx
    movl 16(%ebp), %edx

    movl $4, %eax

    int $0x80
    
    cmpl $0, %eax /* eax >= 0*/
    jge endif 
    /*Si es negativo el error*/
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    jmp endif
    
endif:

    popl %ebx
    popl %ebp

    ret
