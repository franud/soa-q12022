#include <libc.h>

char buff[24];

int pid;

void wasting_time () {
  volatile long long sum = 0;
  for (volatile int i = 0; i < 10000000; ++i) {
    sum += i;
  }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  volatile int x = write(1, "Hola\n", 5);
  if (x == -1) {
    perror();
  }

  volatile int y = gettime ();

  wasting_time ();

  x = write(1, "Done\n", 5);

  volatile int z = gettime ();


  while(1) { }
}
