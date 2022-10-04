/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>
#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
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

/*
fd: file descriptor. In this delivery it must always be 1.
buffer: pointer to the bytes.
size: number of bytes.
return ’ Negative number in case of error (specifying the kind of error) and
the number of bytes written if OK.
*/
int sys_write(int fd, char * buffer, int size)
{
  int error_code = check_fd(fd, ESCRIPTURA);
  if (error_code != 0) {
    return error_code;
  }
  if (buffer == NULL) {
    return -EFAULT;
  }
  if (size < 1) {
    return -EINVAL;
  }

  char dest_buffer [size];
  copy_from_user(buffer, dest_buffer, size);

  int bytes_writen = sys_write_console(dest_buffer, size);

  return bytes_writen;
}