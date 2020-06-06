#pragma once

// ADS1259 Configuration and control commands

#define ADS1259_WAKEUP 0x02 // Wake up from sleep mode
#define ADS1259_SLEEP 0x04 // Begin Sleep Mode
#define ADS1259_RESET 0x06 // Reset to power-up values
#define ADS1259_START_CONV 0x08 // Start Conversion
#define ADS1259_STOP_CONV 0x0A // Stops all conversions after one in progress is complete
#define ADS1259_START_READ_DATA_CONTINOUS 0x10 // enables the Read Data Continuous mode 
#define ADS1259_STOP_READ_DATA_CONTINUOUS 0x11 // cancels the Read Data Continuous mode
#define ADS1259_READ_DATA_BY_OPCODE 0x12 // read the conversion result
#define ADS1259_OFFSET_CALIBRATION 0x18 // performs an offset calibration
#define ADS1259_GAIN_CALIBRATION 0x19 // performs a gain calibration

// ADS1259 READ/WRITE REGISTER COMMANDS
// Each command requires 2 bytes:
// command opcode and register address
// number of registers to read
#define ADS1259_READ_REGISTER 0x20
#define ADS1259_WRITE_REGISTER 0x40

//Samples per second on ADS1259
typedef enum {
    ADS1259_DATA_RATE_10 = 0x0,
    ADS1259_DATA_RATE_17,
    ADS1259_DATA_RATE_50,
    ADS1259_DATA_RATE_60,
    ADS1259_DATA_RATE_400,
    ADS1259_DATA_RATE_1200,
    ADS1259_DATA_RATE_3600,
    ADS1259_DATA_RATE_14400,
    NUM_ADS1259_DATA_RATE,
} Ads1259DataRate;

// REGISTER ADDRESSES
#define ADS1259_ADDRESS_CONFIG0 0x00
#define ADS1259_ADDRESS_CONFIG1 0x01
#define ADS1259_ADDRESS_CONFIG2 0x02
#define ADS1259_ADDRESS_OFC0 0x03
#define ADS1259_ADDRESS_OFC1 0x04
#define ADS1259_ADDRESS_OFC2 0x05
#define ADS1259_ADDRESS_FSC0 0x06
#define ADS1259_ADDRESS_FSC1 0x07
#define ADS1259_ADDRESS_FSC2 0x08
#define NUM_REGISTERS 0x09

// ADS1259 Register Configurations - LSB to MSB
//These values can be changed based on needed configurations

// CONFIG0
#define ADS1259_SPI_TIMEOUT_ENABLE 1 // If 1 SPI timeout enabled
#define ADS1259_INTERNAL_REF_BIAS_ENABLE 1 // If 1 internal ref bias enabled

#define ADS1259_CONFIG0_SETTINGS ((INTERNAL_REF_BIAS_ENABLE<<3) | (SPI_TIMEOUT_ENABLE))

// CONFIG1
#define ADS1259_CONVERSION_DELAY_MS 0 // If 0 Conversion delay disabled
#define ADS1259_VOLTAGE_REFERENCE_SELECT_EXTERNAL 1 // If 1 uses external voltage reference
#define ADS1259_DIGITAL_FILTER_SINC_2 0 // If 0 uses auto SINC1 digital filter (default)
#define ADS1259_CHECK_SUM_ENABLE 1 // If 1 check sum byte appended to output message
#define ADS1259_OUT_OF_RANGE_FLAG_ENABLE 1 // If 1 out of range flag appended to message

#define ADS1259_CONFIG1_SETTINGS ((ADS1259_OUT_OF_RANGE_FLAG_ENABLE<<7) | (ADS1259_CHECK_SUM_ENABLE<<6) | \
    (VOLTAGE_REFERENCE_SELECT_EXTERNAL<<3) | (ADS1259_CONVERSION_DELAY_MS))

// CONFIG2
#define ADS1259_SYNCOUT_ENABLE 0 // If 1 Syncout clock enabled
#define ADS1259_CONVERSION_CONTROL_MODE_PULSE 1 // If 1 conversions set to pulse mode
#define ADS1259_DATA_RATE_SPS DATA_RATE_60 

#define ADS1259_CONFIG2_SETTINGS ((ADS1259_SYNCOUT_ENABLE<<5) | (ADS1259_CONVERSION_CONTROL_MODE_PULSE<<4) | \
            (ADS1259_DATA_RATE_SPS))

//Calibration Register Reset Values
#define ADS1259_OFC0_RESET 0x0
#define ADS1259_OFC1_RESET 0x0
#define ADS1259_OFC2_RESET 0x0
#define ADS1259_FSC0_RESET 0x0
#define ADS1259_FSC1_RESET 0x0
#define ADS1259_FSC2_RESET 0x64