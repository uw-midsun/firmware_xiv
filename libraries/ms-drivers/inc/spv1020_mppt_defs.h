#pragma once

// Internal SPV1020 command definitions and bitmasks.
// Datasheet: https://media.digikey.com/pdf/Data%20Sheets/ST%20Microelectronics%20PDFS/SPV1020.pdf

#define SPV1020_CMD_NOP 0x01
#define SPV1020_CMD_SHUT 0x02
#define SPV1020_CMD_TURN_ON 0x03
#define SPV1020_CMD_READ_CURRENT 0x04
#define SPV1020_CMD_READ_VIN 0x05
#define SPV1020_CMD_READ_PWM 0x06
#define SPV1020_CMD_READ_STATUS 0x07

// Bitmasks for extracting information from the status byte.
#define SPV1020_OVC_MASK 0b1111000
#define SPV1020_OVV_MASK 0b0000100
#define SPV1020_OVT_MASK 0b0000010
#define SPV1020_CR_MASK 0b0000001
