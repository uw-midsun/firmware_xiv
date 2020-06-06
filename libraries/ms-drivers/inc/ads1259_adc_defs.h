#pragma once


// ADS1259 Configuration and control commands

#define ADS_WAKEUP                0x02 // Wake up from sleep mode
#define ADS_SLEEP                 0x04 // Begin Sleep Mode
#define ADS_RESET                 0x06 // Reset to power-up values
#define ADS_START_CONV            0x08 // Start Conversion
#define ADS_STOP_CONV             0x0A // Stops all conversions after one in progress is complete
#define START_READ_DATA_CONTINOUS 0x10 // enables the Read Data Continuous mode 
#define STOP_READ_DATA_CONTINUOUS 0x11 // cancels the Read Data Continuous mode
#define READ_DATA_BY_OPCODE       0x12 // read the conversion result
#define OFFSET_CALIBRATION        0x18 // performs an offset calibration
#define GAIN_CALIBRATION          0x19 // performs a gain calibration

// ADS1259 READ/WRITE REGISTER COMMANDS
// Each command requires 2 bytes:
// command opcode and register address
// number of registers to read
#define READ_REGISTER 0x20
#define READ_REGISTER 0x40


// REGISTER ADDRESSES
#define CONFIG0 0x00
#define CONFIG1 0x01
#define CONFIG2 0x02
#define OFC0    0x03
#define OFC1    0x04
#define OFC2    0x05
#define FSC0    0x06
#define FSC0    0x07
#define FSC0    0x08

// ADS1259 Register Configurations - LSB to MSB
// CONFIG0
#define SPI_TIMEOUT_ENABLE                1 // If 1 SPI timeout enabled
#define INTERNAL_REF_BIAS_ENABLE          1 //If 1 internal ref bias enabled


// CONFIG1
#define CONVERSION_DELAY_MS               0 // If 0 Conversion delay disabled
#define VOLTAGE_REFERENCE_SELECT_EXTERNAL 1 // If 1 uses external voltage reference
#define DIGITAL_FILTER_SINC_2             0 //If 0 uses auto SINC1 digital filter
#define CHECK_SUM_ENABLE                  1 // If 1 check sum byte appended to output message
#define OUT_OF_RANGE_FLAG_ENABLE          1 // If 1 out of range flag appended to message

// CONFIG2
#define SYNCOUT_ENABLE                    0 // If 1 Syncout clock enabled
#define CONVERSION_CONTROL_MODE_PULSE     1 // If 1 conversions set to pulse mode
