#pragma once

// Voltage reference value
#define EXTERNAL_VREF_V 2.5

// ADS1259 Configuration and control commands

#define ADS1259_WAKEUP 0x02      // Wake up from sleep mode
#define ADS1259_SLEEP 0x04       // Begin Sleep Mode
#define ADS1259_RESET 0x06       // Reset to power-up values
#define ADS1259_START_CONV 0x08  // Start Conversion
#define ADS1259_STOP_CONV 0x0A   // Stops all conversions after one in progress is complete
#define ADS1259_START_READ_DATA_CONTINOUS 0x10  // enables the Read Data Continuous mode
#define ADS1259_STOP_READ_DATA_CONTINUOUS 0x11  // cancels the Read Data Continuous mode
#define ADS1259_READ_DATA_BY_OPCODE 0x12        // read the conversion result
#define ADS1259_OFFSET_CALIBRATION 0x18         // performs an offset calibration
#define ADS1259_GAIN_CALIBRATION 0x19           // performs a gain calibration

// ADS1259 READ/WRITE REGISTER COMMANDS
// Each command requires 2 bytes:
// command opcode and register address
// number of registers to read
#define ADS1259_READ_REGISTER 0x20
#define ADS1259_WRITE_REGISTER 0x40

// Samples per second on ADS1259
typedef enum Ads1259DataRate {
  ADS1259_DATA_RATE_10 = 0,
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
#define NUM_ADS1259_REGISTERS 0x09

// ADS1259 Register Configurations - LSB to MSB
// These values can be changed based on needed configurations

// CONFIG0
#define ADS1259_SPI_TIMEOUT_ENABLE 0x01        // Enable SPI Timeout
#define ADS1259_INTERNAL_REF_BIAS_ENABLE 0x04  // Enable internal ref bias

// CONFIG1
#define ADS1259_CONVERSION_DELAY_MS 0x0  // Conversion delay not used -> if needed see datasheet
#define ADS1259_VREF_EXTERNAL 0x08       // Enable external voltage reference
#define ADS1259_DIGITAL_FILTER_2 0x10    // Enable SINC2 digital filter
#define ADS1259_CHECK_SUM_ENABLE 0x40    // Enable check sum byte
#define ADS1259_OUT_OF_RANGE_FLAG_ENABLE 0x80  // Enable out of range flag

// CONFIG2
#define ADS1259_SYNCOUT_ENABLE 0x20                 // Enable Syncout clock
#define ADS1259_CONVERSION_CONTROL_MODE_PULSE 0x10  // Set Conversion mode to pulse
#define ADS1259_DATA_RATE_SPS ADS1259_DATA_RATE_60

// Offset used in Checksum calculation
#define ADS1259_CHECKSUM_OFFSET 0x9B

typedef enum Ads1259RxByte {
  ADS1259_MSB = 0,
  ADS1259_MID,
  ADS1259_LSB,
  ADS1259_CHK_SUM,
  NUM_ADS_RX_BYTES,
} Ads1259RxByte;

#define NUM_CONFIG_REGISTERS 3
#define NUM_REGISTER_WRITE_COMM 5
#define CHK_SUM_FLAG_BIT 0x80
#define RX_NEG_VOLTAGE_BIT 0x800000
#define RX_MAX_VALUE 0x1000000
