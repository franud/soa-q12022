#include <asm.h>

ENTRY (addAsm)
    endbr32 
    push   %ebp
    mov    %esp,%ebp
    mov    0xc(%ebp),%edx
    mov    0x8(%ebp),%eax
    add    %edx,%eax
    pop    %ebp
    ret
