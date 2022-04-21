#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// This should be the COM1 serial port.
#define SERIAL_PORT 0x3F8


void serial_init(uint16_t divisor);
void serial_send_msg(char *msg);


#endif