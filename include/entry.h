/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void keyboard_handler();
void clock_handler();
void system_call_handler();
void syscall_handler_sysenter();

void writeMSR (unsigned int msr_register_code, unsigned int value);

#endif  /* __ENTRY_H__ */
