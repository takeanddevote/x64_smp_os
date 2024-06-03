#include "driver/tty.h"
#include "linux/asm.h"

#define SERIAL_PORT 0x3F8

void init_serial() {
    outByte(SERIAL_PORT + 1, 0x00);    // Disable all interrupts
    outByte(SERIAL_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outByte(SERIAL_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outByte(SERIAL_PORT + 1, 0x00);    //                  (hi byte)
    outByte(SERIAL_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outByte(SERIAL_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outByte(SERIAL_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
    return inByte(SERIAL_PORT + 5) & 1;
}

char read_serial() {
    while (serial_received() == 0);

    return inByte(SERIAL_PORT);
}

int is_transmit_empty() {
    return inByte(SERIAL_PORT + 5) & 0x20;
}

void write_serial(char a) {
    while (is_transmit_empty() == 0);

    outByte(SERIAL_PORT, a);
}

void serial_printk(const char *str, u32 count) {
    for (size_t i = 0; i < count; i++) {
        write_serial(str[i]);
    }
}
