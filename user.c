#include <libc.h>
#include <types.h>

char buff[24];

int pid;

// write tests
// TEST 1: Escribir Hola en pantalla.
void test_write1() {
    write(1, "\n", 1);
    write(1, "hola", 5);
}

// TEST 2: Forzar error: fd != 1.
void test_write2() {
    write(1, "\n", 1);
    write(2, "hola", 5);
    perror();
}


// TEST 3: Forzar error: buffer == NULL
void test_write3() {
    write(1, "\n", 1);
    write(2, NULL, 5);
    perror();
}

// TEST 4: Forzar error: size < 0.
void test_write4() {
    write(1, "\n", 1);
    write(2, "hola", -1);
    perror();
}

int __attribute__ ((__section__(".text.main")))
    main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
    test_write1();
    //test_write2();
    //test_write3();
    //test_write4();

    while(1) { }
}
