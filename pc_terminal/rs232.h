#ifndef _LINUX_RS232_H
#define _LINUX_RS232_H

void rs232_open(void);
void rs232_close(void);
int rs232_getchar_nb();
int rs232_getchar();
int rs232_putchar(char c);

#endif /* _LINUX_RS232_H */
