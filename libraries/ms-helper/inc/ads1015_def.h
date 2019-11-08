#pragma once
// This is an internal file for ads1015 module that provides macros mostly for
// register setup.

// Base I2C address(GND) of ADS1015. Summing it with Ads1015Address enum gives
// an actual I2CAddress. From section 8.5.1.1 of the datasheet.
#define ADS1015_I2C_BASE_ADDRESS ((uint8_t)0x48)

// Register address pointers, used for switching between registers.
// From section 8.6.1 of the datasheet.
#define ADS1015_ADDRESS_POINTER_CONV ((uint8_t)0x0)
#define ADS1015_ADDRESS_POINTER_CONFIG ((uint8_t)0x1)
#define ADS1015_ADDRESS_POINTER_LO_THRESH ((uint8_t)0x2)
#define ADS1015_ADDRESS_POINTER_HI_THRESH ((uint8_t)0x3)

// Upper and Lower bytes of Hi_thresh and Lo_thresh registers.
// The most significant bit of both registers has been set accordingly to enable
// ALRT/RDY pin From section 8.6.4 of the datasheet.
#define ADS1015_LO_THRESH_REGISTER_MSB ((uint8_t)0x0)
#define ADS1015_LO_THRESH_REGISTER_LSB ((uint8_t)0x0)
#define ADS1015_HI_THRESH_REGISTER_MSB ((uint8_t)0xFF)
#define ADS1015_HI_THRESH_REGISTER_LSB ((uint8_t)0xFF)

// ********* The following bytes are fields for the config register
// *********************
// ********* From section 8.6.3 of the datasheet
// ****************************************

// Starts single conversion when in powerdown state.
#define ADS1015_START_SINGLE_CONV ((uint8_t)0x1 << 7)
// Does not start a conversion.
#define ADS1015_IDLE ((uint8_t)0x0)

// Bytes for setting channels
#define ADS1015_AIN(channel) (((channel) + 0x4) << 4)
#define ADS1015_AIN_0 ((uint8_t)0x4 << 4)
#define ADS1015_AIN_1 ((uint8_t)0x5 << 4)
#define ADS1015_AIN_2 ((uint8_t)0x6 << 4)
#define ADS1015_AIN_3 ((uint8_t)0x7 << 4)

// These bytes set the FSR of the programmable gain amplifier.
#define ADS1015_PGA_FSR_6144 ((uint8_t)0x0 << 1)  // ±6.144 V
#define ADS1015_PGA_FSR_4096 ((uint8_t)0x1 << 1)  // ±4.096 V (default)
#define ADS1015_PGA_FSR_2048 ((uint8_t)0x2 << 1)  // ±2.048 V
#define ADS1015_PGA_FSR_1024 ((uint8_t)0x3 << 1)  // ±1.024 V
#define ADS1015_PGA_FSR_512 ((uint8_t)0x4 << 1)   // ±0.512 V
#define ADS1015_PGA_FSR_256 ((uint8_t)0x5 << 1)   // ±0.256 V

// Conversion mode (single or continuous)
#define ADS1015_CONVERSION_MODE_CONT ((uint8_t)0x0)
#define ADS1015_CONVERSION_MODE_SINGLE ((uint8_t)0x1)  // default

// Data sampling rate of the adc
#define ADS1015_DATA_RATE_128 ((uint8_t)0x0 << 5)   //  128 SPS
#define ADS1015_DATA_RATE_250 ((uint8_t)0x1 << 5)   //  250 SPS
#define ADS1015_DATA_RATE_490 ((uint8_t)0x2 << 5)   //  490 SPS
#define ADS1015_DATA_RATE_920 ((uint8_t)0x3 << 5)   //  920 SPS
#define ADS1015_DATA_RATE_1600 ((uint8_t)0x4 << 5)  // 1600 SPS
#define ADS1015_DATA_RATE_2400 ((uint8_t)0x5 << 5)  // 2400 SPS
#define ADS1015_DATA_RATE_3300 ((uint8_t)0x6 << 5)  // 3300 SPS

// Time between each conversion in microseconds.
#define ADS1015_CONVERSION_TIME_US_1600_SPS (1000000 / 1600)

// Comparator operating mode
#define ADS1015_COMP_MODE_TRAD ((uint8_t)0x0 << 4)  // default
#define ADS1015_COMP_MODE_WINDOW ((uint8_t)0x1 << 4)

// Polarity of the ALERT/RDY pin (active low or active high)
#define ADS1015_COMP_POL_LOW ((uint8_t)0x0 << 3)
#define ADS1015_COMP_POL_HIGH ((uint8_t)0x1 << 3)

// Latching comparator
#define ADS1015_COMP_LAT_NON_LATCHING ((uint8_t)0x0 << 2)
#define ADS1015_COMP_LAT_LATCHING ((uint8_t)0x1 << 2)

// Comparator queue and disable
#define ADS1015_COMP_QUE_1_CONV ((uint8_t)0x0)  // Assert after one conversion
#define ADS1015_COMP_QUE_2_CONV ((uint8_t)0x1)  // Assert after two conversion
#define ADS1015_COMP_QUE_4_CONV ((uint8_t)0x2)  // Assert after four conversion
#define ADS1015_COMP_QUE_DISABLE_COMP \
  ((uint8_t)0x3)  // Disable comparator and set ALERT/RDY pin to high-impedance

// **************************************************************************************

// Setup for the config register's upper byte
#define ADS1015_CONFIG_REGISTER_MSB(channel)                                 \
  (ADS1015_START_SINGLE_CONV | ADS1015_AIN(channel) | ADS1015_PGA_FSR_4096 | \
   ADS1015_CONVERSION_MODE_SINGLE)

#define ADS1015_CONFIG_REGISTER_MSB_IDLE \
  (ADS1015_IDLE | ADS1015_AIN_0 | ADS1015_PGA_FSR_4096 | ADS1015_CONVERSION_MODE_SINGLE)

// Setup for the config register's lower byte
#define ADS1015_CONFIG_REGISTER_LSB(datarate)                                                    \
  ((datarate) | ADS1015_COMP_MODE_TRAD | ADS1015_COMP_POL_HIGH | ADS1015_COMP_LAT_NON_LATCHING | \
   ADS1015_COMP_QUE_1_CONV)

// These represent the full-scale range of ADS1015 scaling in mVolts.
// They are used for calculating the LSB size, corresponding to PGA settings.
// From section 8.3.3 of the datasheet.
// Mult. by 2 corresponds to the range from -FS to +FS.
#define ADS1015_FSR_6144 (6144 * 2)
#define ADS1015_FSR_4096 (4096 * 2)
#define ADS1015_FSR_2048 (2048 * 2)
#define ADS1015_FSR_1024 (1024 * 2)
#define ADS1015_FSR_512 (512 * 2)
#define ADS1015_FSR_256 (256 * 2)
#define ADS1015_CURRENT_FSR ADS1015_FSR_4096  // make sure PGA is set up accordingly

// The division factor for calculating LSB size from FSR in mVolt
#define ADS1015_NUMBER_OF_CODES (1 << 12)

// This is used for removing 4 lsb's of the conversion register
// as they are not part of the conversion result.
// From section 8.6.2 of the datasheet.
#define ADS1015_NUM_RESERVED_BITS_CONV_REG 4

// This is stored as the reading for any disabled channel.
#define ADS1015_DISABLED_CHANNEL_READING INT16_MAX

#define ADS1015_BITSET_EMPTY 0

#define ADS1015_READ_UNSUCCESSFUL INT16_MIN
