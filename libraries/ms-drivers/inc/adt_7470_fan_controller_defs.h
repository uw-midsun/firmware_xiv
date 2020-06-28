#pragma once

// ADT4740 Configuration and control commands
// Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/ADT7470.pdf

#define ADT7470_CONFIG_REGISTER_1 \
  0x40  // This register contains the STRT bit, Bit 0, which begins the monitoring cycle on the
        // ADT7470.
#define ADT7470_CONFIG_REGISTER_2 \
  0x74  // Writing a 1 to Bit 0 in this register puts the ADT7470 in shutdown mode,
        // which puts the part into a low current onsumption mode

#define ADT7476A_INTERRUPT_STATUS_REGISTER_1 0x41
#define ADT7476A_INTERRUPT_STATUS_REGISTER_2 0x42

#define ADT7470_FAN_MODE_REGISTER_1 0x68  // bits 6 and 7 contorl the mode for PWM2 and PWM1
#define ADT7470_FAN_MODE_REGISTER_2 0x69  // bits 6 and 7 contorl the mode for PWM4 and PWM3

#define ADT7476A_FAN_MODE_REGISTER_1 0x5C  // bits 7:5 control mode - 111 for manual
#define ADT7476A_FAN_MODE_REGISTER_2 0x5D  // bits 7:5 control mode - 111 for manual
#define ADT7476A_FAN_MODE_REGISTER_3 0x5E  // bits 7:5 control mode - 111 for manual

#define ADT7476A_INTERRUPT_MASK_REGISTER_1 0x74
#define ADT7476A_INTERRUPT_MASK_REGISTER_2 0x75

#define ADT7476A_PWM_1 0x30  // default 0xFF
#define ADT7476A_PWM_2 0x31  // default 0xFF
#define ADT7476A_PWM_3 0x32  // default 0xFF

#define ADT7470_MANUAL_MODE_MASK 0b11100000

#define NUM_GPIO_FAN_PINS \
  4  // The ADT7470 has four pins that can be configured as either general-purpose logic pins or as
     // PWM outputs

#define NUM_BYTES_TO_READ 1
#define NUM_BYTES_TO_WRITE 1

#define ADT7470_GPIO_ENABLE_REGISTER \
  0x7F  // To enable the PWM output on the ADT7470 as GPIOs, the enable bits in Register 0x7F must
        // be set to 1. (bits 0 to 3)
#define ADT7470_GPIO_DIRECTION_REGISTER \
  0x80  // Setting a direction bit to 1 in the GPIO configuration register makes the corresponding
        // GPIO pin an output.
        //  Clearing the direction bit to 0 makes it an input.  bits 7, 5, 3, 1
#define ADT7470_GPIO_POLARITY_REGISTER \
  0x80  // Setting a polarity bit to 1 makes the corresponding GPIO pin active high. Clearing the
        // polarity bit to 0 makes it active low.

// how are these read? on the schematic it says I2C address: 0101100_R/W and hex values for read or
// write. assuming those are tagged on after a don't care for a total of 16 bits?

#define I2C1_SDA \
  { .port = GPIO_PORT_B, .pin = 16 }
#define I2C1_SCL \
  { .port = GPIO_PORT_B, .pin = 1 }

#define SET_SPEED_NUM_BYTES 2

// need to create masks for temperature interrupt, since we aren't handling that in this driver
// need to set up interrupt? somehow not sure how it is raised yet.
