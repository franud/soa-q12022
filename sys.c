/*
 * sys.c - Syscalls implementation
 */
#include "errno.h"
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

int sys_write(int fd, char * buffer, int size) {
  int err = check_fd(fd, ESCRIPTURA);
  if (err <= 0) return err; // if check_fd returned an error, return that same error.
  if (buffer == NULL) return -EFAULT; // error out if buffer is null.
  if (size < 1) return -EINVAL; // error out if there is nothing to write (size < 1).

  int bytes_left = size;

  int sys_buffer_size = 256;
  char sys_buffer[sys_buffer_size];

  // write in chunks that fit into our sys_buffer.
  while (bytes_left > sys_buffer_size) {
    int err = copy_from_user(buffer, sys_buffer, sys_buffer_size);
    if (err != 0) return -EIO; // check for errors copying data.

    int bytes_written = sys_write_console(sys_buffer, sys_buffer_size);

    bytes_left -= bytes_written;
    buffer += bytes_written;
  }
  
  // write only the remaining bytes.
  if ( bytes_left > 0 ) {
    int err = copy_from_user(buffer, sys_buffer, bytes_left);
    if (err != 0) return -EIO; // check for errors copying data.

    int bytes_written = sys_write_console(sys_buffer, bytes_left);

    bytes_left -= bytes_written;
    buffer += bytes_written;
  }
  
  return size;
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

