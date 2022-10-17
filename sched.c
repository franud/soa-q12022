/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <interrupt.h>
#include <entry.h>
#include <list.h>
#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;
struct list_head freequeue;
struct list_head readyqueue;
struct task_struct * idle_task;

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
	;
	}
}

void init_idle (void)
{
	// grab list pointer of a free task from freequeue
	struct list_head * idle_list_pointer = list_first(&freequeue);
	// remove it from freeque.
	list_del(idle_list_pointer);
	
	// get it's corresponding PCB.
	struct task_struct * idle_pcb = list_head_to_task_struct(idle_list_pointer);
	
	// set task PID to 0.
	idle_pcb->PID = 0;
	// initalize dir_pages_baseAddr field of the task.
	allocate_DIR(idle_pcb);
	
	// get corresponding task_union.
	union task_union * idle_task_union = (union task_union *) idle_pcb;
	
	/* For compatibility with task_switch, set the stack as follows:
	 * - set the value right above the bottom of the stack to 0.
	 *   (task_switch will pop that value into EBP before returning,
	 *   0 shouldn't cause any conflicts)
	 * - set the bottom of the stack to point to the cpu_idle routine.
	 *   (task_switch should call that routine using a ret instruction)
	 */
	idle_task_union->stack[KERNEL_STACK_SIZE-2] = 0;
	idle_task_union->stack[KERNEL_STACK_SIZE-1] = (unsigned long) cpu_idle;

	/* The previous values are useless if the kernel_esp field doesn't
	 * point at them.
	 * For that reason this task's kernel_esp shall point at the value we
	 * want task_swit*ch to pop into EBP.
	 */
	idle_pcb->kernel_esp = &(idle_task_union->stack[KERNEL_STACK_SIZE-2]);

	// set global variable idle_task to point at the now initialized idle PCB
	idle_task = idle_pcb;
}

void init_task1(void)
{
	struct list_head * init_list_pointer = list_first(&freequeue);
	list_del(init_list_pointer);

	struct task_struct * init_pcb = list_head_to_task_struct(init_list_pointer);
	init_pcb->PID = 1;
	allocate_DIR(init_pcb);

	set_user_pages(init_pcb);

	union task_union * init_task_union = (union task_union *) init_pcb;

	tss.esp0 = KERNEL_ESP(init_task_union);

	writeMSR(0x175, (unsigned int) tss.esp0);

	set_cr3(init_pcb->dir_pages_baseAddr);

}


void init_sched()
{
	INIT_LIST_HEAD(&freequeue);

	struct list_head * last_head = &freequeue;

	for (int i = 0; i < NR_TASKS; ++i) {
		struct list_head * new = &task[i].task.list;
		list_add(new, last_head);
		last_head = new;
	}

	INIT_LIST_HEAD(&readyqueue);
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

