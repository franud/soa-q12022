/*
 * sys.c - Syscalls implementation
 */
#include <errno.h>
#include <types.h>
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
    if (fd!=1) return -9; /*EBADF*/
    if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
    return 0;
}

extern int zeos_ticks;

int next_pid = 2;

#define BUFFER_SIZE 256
char write_buffer[BUFFER_SIZE];

int ret_from_fork () {
    return 0;
}

int sys_write(int fd, char * buffer, int size) {
    int err = check_fd(fd, ESCRIPTURA);
    if (err) return err; // if check_fd returned an error, return that same error.
    if (buffer == NULL) return -EFAULT; // error out if buffer is null.
    if (size < 1) return -EINVAL; // error out if there is nothing to write (size < 1).

    int bytes = size;
    int bytes_written;
  
    while(bytes > BUFFER_SIZE){
        copy_from_user(buffer+(size-bytes), write_buffer, BUFFER_SIZE);
        bytes_written = sys_write_console(write_buffer, BUFFER_SIZE);
            
        buffer = buffer+BUFFER_SIZE;
        bytes = bytes-bytes_written;
    }
    
    // Copy any leftover bytes
    copy_from_user(buffer+(size-bytes), write_buffer, bytes);
    bytes_written = sys_write_console(write_buffer, bytes);
    bytes = bytes-bytes_written;    
    
    return size-bytes;
  }
int sys_gettime() {
    return zeos_ticks;
}


int sys_ni_syscall()
{
    return -38; /*ENOSYS*/
}

int sys_getpid()
{
    return current()->PID;
}

int sys_fork()
{

    /*a) Get a free task_struct for the process. If there is no space for a new process, an error will be returned. */
    if (list_empty (&freequeue)) {
        return -EAGAIN;
    }
    struct list_head * child_list_entry = list_first(&freequeue);
    list_del(child_list_entry);

    /*b) Inherit system data: copy the parent’s task_union to the child.*/
    struct task_struct * child_pcb = list_head_to_task_struct(child_list_entry);
    struct task_struct * parent_pcb = current();

    union task_union * child_union = (union task_union *) child_pcb;
    union task_union * parent_union = (union task_union *) parent_pcb;        
    copy_data(parent_union, child_union, sizeof(union task_union));

    /* c) Initialize field dir_pages_baseAddr with a new directory to store the process address space using the allocate_DIR routine. */ 

    allocate_DIR(child_pcb);

    /* d) Search physical pages in which to map logical pages for data+stack of the child process (using the alloc_frames function). If there is no enough free pages, an error will be return. */
    int allocated_frame;
    int availaible_frames [NUM_PAG_DATA];
    for (int i = 0; i < NUM_PAG_DATA; ++i) {
        allocated_frame = alloc_frame();
        availaible_frames[i] = allocated_frame;
        if (allocated_frame == -1) {
            for (int j = 0; j < i; ++j) {
                free_frame(availaible_frames[j]);
            }
            list_add_tail(&child_pcb->list, &freequeue);
            return -ENOMEM;
        }
    }

    /* e.i) Create new address space: Access page table of the child process through the directory field in the task_struct to initialize it (get_PT routine can be used) */
    page_table_entry * child_page_table = get_PT(child_pcb);
    page_table_entry * parent_page_table = get_PT(parent_pcb);
    
    /*e.i.A) Page table entries for the system code and data and for the user code can be a copy of the page table entries of the parent process (they will be shared)*/
    for (int i = 1; i < NUM_PAG_KERNEL; i++) {
        set_ss_pag(child_page_table, i, get_frame(parent_page_table, i));
    }

    for (int i = PAG_LOG_INIT_CODE; i < PAG_LOG_INIT_CODE + NUM_PAG_CODE; ++i) {
        set_ss_pag(child_page_table, i, get_frame(parent_page_table, i));
    }

    /*e.i.B) Page table entries for the user data+stack have to point to new allocated pages which hold this region */
    for (int i = 0; i < NUM_PAG_DATA; i++)
    {
        set_ss_pag(child_page_table, PAG_LOG_INIT_DATA + i, availaible_frames[i]);
    }

    /*e.ii) Copy the user data+stack pages from the parent process to the child process.*/

    for (int i = 0; i < NUM_PAG_DATA; ++i) {
        /*e.ii.A) Use temporal free entries on the page table of the parent. Use the set_ss_pag and del_ss_pag functions.*/
        set_ss_pag(parent_page_table, PAG_LOG_INIT_DATA + NUM_PAG_DATA + i, availaible_frames[i]);
        /*e.ii.B) Copy data+stack pages.*/
        copy_data((PAG_LOG_INIT_DATA + i) << 12, (PAG_LOG_INIT_DATA + NUM_PAG_DATA + i) << 12, PAGE_SIZE);
        /*e.ii.C) Free temporal entries in the page table and ...*/
        del_ss_pag(parent_page_table, PAG_LOG_INIT_DATA + NUM_PAG_DATA + i);
    }

    /* e.ii.C) flush the TLB to really disable the parent process to access the child pages.*/
    set_cr3(get_DIR(parent_pcb));

    /*f) Assign a new PID to the process. The PID must be different from its position in the task_array table.*/
    child_pcb->PID = next_pid;          
    ++next_pid;

    /*g) Initialize the fields of the task_struct that are not common to the child. */


    /*  
      $0              <- 0x13
      @ret_from_fork  <- 0x12
      @ret_to_handler <- 0x11 <- (syscall_handler)
      %ebx
      %ecx
      %edx
      %esi
      %edi
      %ebp
      %eax
      %ds
      %es
      %fs
      %gs
      %eip
      %cs
      %eflags
      %esp
      %ss           
    */
    DWord * child_kernel_esp = (DWord *) KERNEL_ESP(child_union);
    child_kernel_esp[-0x13] = 0;
    child_kernel_esp[-0x12] = ret_from_fork;
    child_pcb->kernel_esp = &child_kernel_esp[-0x13];
    
    /*i) Insert the new process into the ready list: readyqueue. This list will contain all processes that are ready to execute but there is no processor to run them. */
    list_add(&child_pcb->list, &readyqueue);

    /*j) Return the PID of the child process 👶*/
    return child_pcb->PID;
}

void sys_exit()
{  
    /*Free the data structures and resources of this process (physical memory, task_struct, and so). It uses the free_frame function to free physical pages.*/
    page_table_entry * current_page_table = get_PT(current());

    for (int i = PAG_LOG_INIT_DATA; i < NUM_PAG_DATA; ++i) {
        free_frame(get_frame(current_page_table, i));
        del_ss_pag(current_page_table, i);
    }

    {
        current()->PID = -1;
        current()->dir_pages_baseAddr = NULL;
        current()->kernel_esp = NULL;
        current()->quantum = 0;
    }

    list_add_tail(&(current()->list),&freequeue);

    sched_next_rr();
}

