#include <asm.h>


/* void task_switch(union task_union* new) */
ENTRY(task_switch)
    push   %ebp
    mov    %esp,%ebp
    push   %esi
    push   %edi
    push   %ebx

    pushl  0x8(%ebp)
    call   inner_task_switch
    add    $0x4,%esp

    pop    %ebx
    pop    %edi
    pop    %esi
    pop    %ebp
    ret

/*void swap_stacks (DWord * current_kernel_esp0, DWord * new_kernel_esp0) 
    4(%esp) -> current_kernel_esp0: puntero al campo kernel_esp dentro del task_struct del proceso actual y al que asignaremos el %ebp actual
    8(%esp) -> new_kernel_esp0: es el campo kernel_esp del task_struct del nuevo proceso y asignaremos %esp a new_kernel_esp0 para que apunte a la pila del nuevo proceso
    No hacemos push %ebp ni movl %esp, %ebp ya que precisamente queremos mantener el %ebp de inner_task_switch
*/
ENTRY(swap_stacks)
    movl 4(%esp), %ebx
    movl %ebp, (%ebx)
    movl 8(%esp), %esp
    pop %ebp
    ret