/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

extern int current_quantum_ticks;

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;
  DWord * kernel_esp;
  int quantum;
  enum state_t state;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */

extern struct list_head freequeue;
void init_freequeue();

extern struct list_head readyqueue;
void init_readyqueue();

/*
5) Define a global variable idle_task
*/
extern struct task_struct * idle_task;

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union * new);

void inner_task_switch(union task_union * new);

void swap_stacks (DWord * current_kernel_esp0, DWord * new_kernel_esp0);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */

/*Function to select the next process to execute, to extract it from the ready queue and to invoke the context switch process. 
This function should always be executed after updating the state of the current process (after calling function update_process_state_rr).
*/
void sched_next_rr();

/* 
Function to update the current state of a process to a new state. 
This function deletes the process from its current queue (state) and inserts it into a new queue (for example, the free queue or the ready queue).
If the current state of the process is running, then there is no need to delete it from any queue.
The parameters of this function are the task_struct of the process and the queue according to the new state of the process.
If the new state of the process is running, then the queue parameter should be NULL.
*/
void update_process_state_rr(struct task_struct *t, struct list_head *dest);

/*Function to decide if it is necessary to change the current process.
returns: 1 if it is necessary to change the current process and 0 otherwise
*/
int needs_sched_rr();

/*Function to update the relevant information to take scheduling decisions. In the case of the round robin policy it should update the number of ticks that the process has executed since it got assigned the cpu.*/
void update_sched_data_rr();


/* System scheduler. Must be called from clock_routine.
 *
 */
void schedule();

int get_quantum (struct task_struct *t);
void set_quantum (struct task_struct *t, int new_quantum);


#endif  /* __SCHED_H__ */
