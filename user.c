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
  write (1, "Hola! Estamos en el user.c\n", 28);
  while(1) { }
}
