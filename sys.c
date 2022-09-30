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

#define BUFFER_SIZE 256
char write_buffer[BUFFER_SIZE];

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
    int PID=-1;

    // creates the child process
    
    return PID;
}

void sys_exit()
{  
}

