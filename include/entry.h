/*
 * entry.h - Definició del punt d'entrada de les crides al sistema
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

void clock_handler();
void keyboard_handler();
void system_call_handler();
void syscall_handler_sysenter();

// Write a value to the desired MSR position.
void writeMSR(unsigned int msr_position, unsigned int value);

#endif  /* __ENTRY_H__ */
