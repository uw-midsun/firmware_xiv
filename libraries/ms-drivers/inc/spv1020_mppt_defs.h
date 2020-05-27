#pragma once

// Internal SPV1020 command definitions and bitmasks.
// Datasheet: https://media.digikey.com/pdf/Data%20Sheets/ST%20Microelectronics%20PDFS/SPV1020.pdf

#define COMMAND_NOP          0x01
#define COMMAND_SHUT         0x02
#define COMMAND_TURN_ON      0x03
#define COMMAND_READ_CURRENT 0x04
#define COMMAND_READ_VIN     0x05
#define COMMAND_READ_PWM     0x06
#define COMMAND_READ_STATUS  0x07

// These bitmasks assume the status register is 7 bits - 4 bits OVC, 1 bit OVV, 1 bit OVT, 1 bit CR.
// An alternate reading of the datasheet could suggest 13 bits, with 7 CR bits. Ask Micah Black pls.
#define OVC_BITMASK 0b1111000
#define OVV_BITMASK 0b0000100
#define OVT_BITMASK 0b0000010
#define CR_BITMASK  0b0000001
