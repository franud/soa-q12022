/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void scroll () {
  Word *screen = (Word *)0xb8000;
  for (Byte j = 1; j < NUM_ROWS; ++j) {
    for (Byte i = 0; i < NUM_COLUMNS; ++i) {
      screen [( (j - 1) * NUM_COLUMNS + i )] = screen [ (j * NUM_COLUMNS + i) ];
    }
  }

  for (Byte i = 0; i < NUM_COLUMNS; ++i) {
    screen [(NUM_ROWS - 1) * NUM_COLUMNS + i] = 0x0000;
  }

  y = NUM_ROWS - 1;
  x = 0;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    scroll ();
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
    Word *screen = (Word *)0xb8000;
    screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      scroll();
    }
  }
}

void printc_color(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x7000;  /* Grey bg and black letters */
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}

void printk_black_grey(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc_color(string[i]);
}
