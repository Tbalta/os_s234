#include "serial.h"

#include "port.h"
#include <stdbool.h>

void serial_init(uint16_t divisor)
{
    // Set DLAB to 1
    outb(SERIAL_PORT + 3, 0x80);

    // Set the divisor
    outb(SERIAL_PORT + 0, divisor & 0x0F);
    outb(SERIAL_PORT + 1, divisor & 0xF0);

    outb(SERIAL_PORT + 3, 0);
    outb(SERIAL_PORT + 3, 0x03); // 8 bits, no parity, one stop bit

    // Enable fifo, with dma.
    outb(SERIAL_PORT + 2, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (0b11 << 6));
}

bool is_fifo_empty()
{
    return inb(SERIAL_PORT + 5) & (1 << 6);
}

void serial_send_msg(char *msg)
{
    // Wait for fifo to be empty for synchronisation
    while (!is_fifo_empty())
        continue;
    for (int i = 0; msg[i] != '\0'; i++)
    {
        // Fifo buffer is 14 char long so we need to wait before putting the funf char.
        if (i % 14 == 0)
            while (!is_fifo_empty())
                ;
        outb(SERIAL_PORT, msg[i]);
    }
}