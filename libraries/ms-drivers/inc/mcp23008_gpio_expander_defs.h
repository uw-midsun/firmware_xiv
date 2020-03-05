#pragma once

// Internal MCP23008 register definitions
// Datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/21919e.pdf

#define IODIR 0x00    // direction, 1 input 0 output
#define IPOL 0x01     // polarity, corresponding GPIO bit will show inverted value on the pin
#define GPINTEN 0x02  // interrupt on change 1 enable 0 disable
#define DEFVAL 0x03   // default compare value for interrupt, opposite value on pin causes interrupt
#define INTCON 0x04   // interrupt control, interrupt-on-change vs interrupt-on-previous-val
#define IOCON 0x05    // IO configuration register

#define GPPU 0x06    // pull-up-resistor 1 pulled up
#define INTF 0x07    // Interrupt flag, 1 means that pin caused interrupt
#define INTCAP 0x08  // captures the GPIO PORT value when interrupt occurs
#define GPIO 0x09    // read GPIO from here, write modifies the Output Latch
#define OLAT 0x0A    // 'read' reads OLAT, write modifies the pins configured as output
