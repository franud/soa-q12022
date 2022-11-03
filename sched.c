/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include "list.h"
#include "sched.h"
#include "mm.h"
#include "io.h"
#include "entry.h"

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	while(1)
	{
		printk("idle\n");
	}
}

void init_idle (void)
{
	/* 1) Get an available task_union from the freequeue to contain the characteristics of this process. 
	FRAN: Sabemos que el primer elemento es el list_head de task[0] porque se han inicializado en orden [0..NR_TASKS-1].
	*/
	struct list_head * first_queue_element = list_first(&freequeue);
	struct task_struct * idle_pcb = list_head_to_task_struct(first_queue_element);
	/*FRAN: Hay que quitar first_queue_element del freequeue? yo creo que sí*/
	list_del(first_queue_element);
	/* 2) Assign PID 0 to the process. */
	idle_pcb->PID = 0;
	/* 3) Initialize field dir_pages_baseAaddr with a new directory to store the process address space using the allocate_DIR routine.*/
	allocate_DIR(idle_pcb);
	/* 4) Initialize an execution context for the procees to restore it when it gets assigned the cpu (see section 4.5) and executes cpu_idle.*/
	union task_union * idle_union = (union task_union *) idle_pcb;

	/*Store in the stack of the idle process the address of the code that it will execute (address of the cpu_idle function). */
	idle_union->stack[KERNEL_STACK_SIZE - 1] = (DWord) &cpu_idle;

	/*Store in the stack the initial value that we want to assign to register ebp when undoing the dynamic link (it can be 0)
	FRAN: Es 0x41414141 porque es "AAAA" en ascii y me ayuda verlo en gdb. */
	idle_union->stack[KERNEL_STACK_SIZE - 2] = 0x41414141;

	/* Finally, we need to keep (in a field of its task_struct) the position of the stack where we have stored the initial value for the ebp register. This value will be loaded in the esp register when undoing the dynamic link.
	*/
	idle_pcb->kernel_esp = &idle_union->stack[KERNEL_STACK_SIZE - 2];

	/* 6) Initialize the global variable idle_task, which will help to get easily the task_struct of the idle process. */
	idle_task = idle_pcb;
}	

void init_task1(void)
{
	struct list_head * first_queue_element = list_first(&freequeue);
	struct task_struct * task1_pcb = list_head_to_task_struct(first_queue_element);

	list_del(first_queue_element);

	/* 1) Assign PID 1 to the process */
	task1_pcb->PID = 1;
	/* 2) Initialize field dir_pages_baseAaddr with a new directory to store the process address space using the allocate_DIR routine. */
	allocate_DIR(task1_pcb);

	/* 3) Complete the initialization of its address space, by using the function set_user_pages (see file mm.c). This function allocates physical pages to hold the user address space (both code and data pages) and adds to the page table the logical-to-physical translation for these pages. Remind that the region that supports the kernel address space is already configure for all the possible processes by the function init_mm. */
	set_user_pages(task1_pcb);

	/* 4) Update the TSS to make it point to the new_task system stack. In case you use sysenter you must modify also the MSR number 0x175.
	FRAN: Aquí tener cuidado con el sysenter
	*/
	union task_union * task1_union = (union task_union *) task1_pcb;

	DWord * task1_stack_base = &task1_union->stack[KERNEL_STACK_SIZE];

	tss.esp0 = (DWord) task1_stack_base;
	writeMSR(0x175, (unsigned int) task1_stack_base);

	/* 5) Set its page directory as the current page directory in the system, by using the set_cr3 function (see file mm.c). */
	set_cr3(task1_pcb->dir_pages_baseAddr);

}

void inner_task_switch(union task_union * new) {
	/*1) Update the pointer to the system stack to point to the stack of new_task. This step depends on the implemented mechanism to enter the system. In the case that the int assembly instruction is used to invoke the system code, TSS.esp0 must be modified to make it point to the stack of new_task. If the system code is invoked using sysenter, MSR number 0x175 must be also modified. */
	/*FRAN: Tener cuidado con lo que dijo el profe de tener solo o int 0x80 o sysenter, quizá alguno de esos dos falta, aunque realmente int 0x80 no es la única interrupción así que creo que deberíamos dejarlo.
	FRAN: Tiene que apuntar a la base del stack o al esp del stack?
	*/
	DWord * current_kernel_esp0 = &(current()->kernel_esp);
	DWord * new_kernel_esp0 = new->task.kernel_esp;

	DWord * new_stack_base = &(new->stack[KERNEL_STACK_SIZE]);

	tss.esp0 = (DWord) new_stack_base;
	writeMSR(0x175, (unsigned int) new_stack_base);

	/*2) Change the user address space by updating the current page directory: use the set_cr3 funtion to set the cr3 register to point to the page directory of the new_task. */
	set_cr3 (get_DIR(&new->task));

	/*3) Store the current value of the EBP register in the PCB. EBP has the address of the current system stack where the inner_task_switch routine begins (the dynamic link). */
	/*4) Change the current system stack by setting ESP register to point to the stored value in the new PCB */
	swap_stacks(current_kernel_esp0, new_kernel_esp0);
	
	/*FRAN: Aquí se supone que debería haber un pop de %ebp y un ret pero nunca llegamos a él*/
}

int get_quantum (struct task_struct *t) {
	return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum) {
	t->quantum = new_quantum;
}

void update_sched_data_rr() {
	--current_quantum_ticks;	
}

int needs_sched_rr() {
	/* If quantum >= 0 we still don't have to switch tasks. */
	if (current_quantum_ticks > 0)
		return 0;
	/*  If quantum has reached zero, but there are no more available
	 *  processes in the ready queue, there is no task switch and we 
	 *  reset the quantum.
	 */ 
	else if (list_empty(&readyqueue)) {
		current_quantum_ticks = get_quantum(current());
		return 0;
	}
	/* if quantum has reached zero and there are available processes,
	 * it's time to switch tasks.
	 */
	else {
		return 1;
	}
}

enum state_t get_queue_state (struct list_head * list) {
	if (list == &readyqueue) {
		return ST_READY;
	}
	return ST_BLOCKED;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest) {
	enum state_t state = t->state;
	
	if (state != ST_RUN) {
		list_del(&t->list);
	}
	
	/* if dest isn't a pointer to a queue; 
	 * new process state is run 
	 */ 
	if (dest == NULL) { // t is the new running process
		t->state = ST_RUN;
		return;
	}

	list_add_tail(&t->list, dest);
	/* update state */

	t->state = get_queue_state(dest);
	
}

void sched_next_rr() {
	struct task_struct * next_process;

	if (list_empty(&readyqueue)) {
		next_process = idle_task;
	} else {
		struct list_head * next = list_first(&readyqueue);
		next_process = list_head_to_task_struct(next);
	}
	current_quantum_ticks = get_quantum(next_process);

	task_switch((union task_union *) next_process);

}

void sched() {
	update_sched_data_rr();
	if (needs_sched_rr()) {
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}

void init_sched()
{
	init_readyqueue();
	init_freequeue();
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void init_freequeue () 
{
	INIT_LIST_HEAD(&freequeue);

	for (int i = 0; i < NR_TASKS; ++i)
	{
		struct list_head* list_of_ith_task = &task[i].task.list;

		list_add(list_of_ith_task, &freequeue);
	}
}

void init_readyqueue ()
{
	INIT_LIST_HEAD(&readyqueue);
}
