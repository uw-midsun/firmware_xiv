#pragma once

// Internal PCA9539R register definitions
// Datasheet: https://www.nxp.com/docs/en/data-sheet/PCA9539_PCA9539R.pdf

#define INPUT0 0x00   // inputs from GPIO pins 0_0 (LSB) to 0_7 (MSB)
#define INPUT1 0x01   // inputs from GPIO pins 1_0 (LSB) to 1_7 (MSB)
#define OUTPUT0 0x02  // outputs to GPIO pins 0_0 (LSB) to 0_7 (MSB)
#define OUTPUT1 0x03  // outputs to GPIO Pins 1_0 (LSB) to 1_7 (MSB)

// 0 is don't invert, 1 is invert
#define IPOL0 0x04  // invert polarity of INPUT0 data
#define IPOL1 0x05  // invert polarity of INPUT1 data

// 0 is output, 1 is input, reset sets to inputs
#define IODIR0 0x06  // IO direction of 0_0 (LSB) to 0_7 (MSB)
#define IODIR1 0x07  // IO direction of 1_0 (LSB) to 1_7 (MSB)
