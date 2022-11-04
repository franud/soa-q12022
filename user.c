#include <libc.h>
#include <types.h>

char buff[32];

int pid;

// write tests
// TEST 1: Escribir Hola en pantalla.
void test_write1() {
    write(1, "\n", 1);
    write(1, "hola\n", 5);
}

// TEST 2: Forzar error: fd != 1.
void test_write2() {
    write(1, "\n", 1);
    write(2, "hola\n", 5);
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
    write(2, "hola\n", -1);
    perror();
}

// gettime test: print time elapsed.
void test_gettime() {
    itoa(gettime(), buff);
    write(1, buff, strlen(buff));
    write(1, "\n", 1);
}

/* 1 padre tiene un 1 hijo... fin.*/
void test_fork1() {
    int fork_ret = fork ();
    int pid = getpid();
    char buffer [2];
    itoa(pid, buffer);
    if (fork_ret == 0) {
        write(1, "soy hijo\n", 10);
        write(1, buffer, 2);
        write(1, "\n", 1);
    }
    if (fork_ret > 0) {
        write(1, "soy papi chulo\n", 16);
        write(1, buffer, 2);
        write(1, "\n", 1);
    }
}


void test_exit1 () {
    int fork_ret = fork ();
    int pid = getpid();
    char buffer [2];
    itoa(pid, buffer);
    if (fork_ret == 0) {
        write(1, "soy hijo\n", 10);
        write(1, buffer, 2);
        write(1, "\n", 1);
    }
    if (fork_ret > 0) {
        write(1, "soy papi chulo\n", 16);
        write(1, buffer, 2);
        write(1, "\n", 1);
        exit();
    }

    write(1, "soy el hijo mi padre ha muerto\n", 32);
    write(1, buffer, 2);
    write(1, "\n", 1);

    write(1, "me voy a morir yo tambien de depresion\nMe remplazara el idle...\n", 65);
    exit();
}

/* Hacer test de que cuando se crean más de NR_TASK procesos, se lanza error y perror muestra el error de ENOMEM*/
void test_fork2 () {
    int ret = 1;
    for (int i = 0; i < 10; ++i) {
        if (ret > 0) {
            ret = fork();
        }
        if (ret == -1) {
            perror();
        }
    }
}

int __attribute__ ((__section__(".text.main")))
    main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    //test_write1();
    //test_write2();
    //test_write3();
    //test_write4();
    //test_fork1();
    //test_exit1();
    test_fork2();
    
    while(1) {
        //write(1, buffer_pid, 2);
        //test_gettime();
    }
}
