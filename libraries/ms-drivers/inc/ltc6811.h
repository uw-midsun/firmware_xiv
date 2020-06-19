#pragma once
#include <assert.h>

#include "ltc_afe.h"

// used internally by the LTC AFE driver

#define LTC6811_CELLS_IN_REG 3
#define LTC6811_GPIOS_IN_REG 3
#define ADS1259_SHIFT_REG_SIZE 8
#define ADS1259_NUM_PINS 32

typedef enum {
  LTC_AFE_REGISTER_CONFIG = 0,
  LTC_AFE_REGISTER_CELL_VOLTAGE_A,
  LTC_AFE_REGISTER_CELL_VOLTAGE_B,
  LTC_AFE_REGISTER_CELL_VOLTAGE_C,
  LTC_AFE_REGISTER_CELL_VOLTAGE_D,
  LTC_AFE_REGISTER_AUX_A,
  LTC_AFE_REGISTER_AUX_B,
  LTC_AFE_REGISTER_STATUS_A,
  LTC_AFE_REGISTER_STATUS_B,
  LTC_AFE_REGISTER_READ_COMM,
  LTC_AFE_REGISTER_START_COMM,
  NUM_LTC_AFE_REGISTERS
} LtcAfeRegister;

typedef enum {
  LTC_AFE_VOLTAGE_REGISTER_A = 0,
  LTC_AFE_VOLTAGE_REGISTER_B,
  LTC_AFE_VOLTAGE_REGISTER_C,
  LTC_AFE_VOLTAGE_REGISTER_D,
  NUM_LTC_AFE_VOLTAGE_REGISTERS
} LtcAfeVoltageRegister;

typedef enum {
  LTC_AFE_DISCHARGE_TIMEOUT_DISABLED = 0,
  LTC_AFE_DISCHARGE_TIMEOUT_30_S,
  LTC_AFE_DISCHARGE_TIMEOUT_1_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_2_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_3_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_4_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_5_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_10_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_15_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_20_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_30_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_40_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_60_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_75_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_90_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_120_MIN
} LtcAfeDischargeTimeout;

// SPI Packets
typedef struct {
  uint8_t adcopt : 1;
  uint8_t swtrd : 1;
  uint8_t refon : 1;

  uint8_t gpio : 5;  // GPIO pin control

  uint32_t undervoltage : 12;  // Undervoltage Comparison Voltage
  uint32_t overvoltage : 12;   // Overvoltage Comparison Voltage

  uint16_t discharge_bitset : 12;
  uint8_t discharge_timeout : 4;
} _PACKED LtcAfeConfigRegisterData;
static_assert(sizeof(LtcAfeConfigRegisterData) == 6, "LtcAfeConfigRegisterData must be 6 bytes");

// COMM Register, refer to LTC6803 datasheet page 31, Table 15
typedef struct {
  // COMM0
  uint8_t icom0 : 4;
  uint8_t d0 : 8;
  uint8_t fcom0 : 4;

  uint8_t icom1 : 4;
  uint8_t d1 : 8;
  uint8_t fcom1 : 4;

  uint8_t icom2 : 4;
  uint8_t d2 : 8;
  uint8_t fcom2 : 4;
} _PACKED LtcAfeCommRegisterData;
static_assert(sizeof(LtcAfeCommRegisterData) == 6, "LtcAfeCommRegisterData must be 6 bytes");

// CFGR packet
typedef struct {
  LtcAfeConfigRegisterData reg;

  uint16_t pec;
} _PACKED LtcAfeWriteDeviceConfigPacket;

// WRCOMM + mux pin
typedef struct {
  uint8_t wrcomm[4];
  LtcAfeCommRegisterData reg;
} _PACKED LtcAfeWriteCommRegPacket;

// WRCFG + all slave registers
typedef struct {
  uint8_t wrcfg[4];

  // devices are ordered with the last slave first
  LtcAfeWriteDeviceConfigPacket devices[LTC_AFE_MAX_CELLS_PER_DEVICE];
} _PACKED LtcAfeWriteConfigPacket;
#define SIZEOF_LTC_AFE_WRITE_CONFIG_PACKET(devices) \
  (4 + (devices) * sizeof(LtcAfeWriteConfigPacket))

typedef union {
  uint16_t voltages[3];

  uint8_t values[6];
} LtcAfeRegisterGroup;
static_assert(sizeof(LtcAfeRegisterGroup) == 6, "LtcAfeRegisterGroup must be 6 bytes");

typedef struct {
  LtcAfeRegisterGroup reg;

  uint16_t pec;
} _PACKED LtcAfeVoltageRegisterGroup;
static_assert(sizeof(LtcAfeVoltageRegisterGroup) == 8,
              "LtcAfeVoltageRegisterGroup must be 8 bytes");

typedef struct {
  LtcAfeRegisterGroup reg;

  uint16_t pec;
} _PACKED LtcAfeAuxRegisterGroupPacket;
static_assert(sizeof(LtcAfeAuxRegisterGroupPacket) == 8,
              "LtcAfeAuxRegisterGroupPacket must be 8 bytes");

// command codes
// see Table 38 (p.59)
#define LTC6811_WRCFG_RESERVED (1 << 0)

#define LTC6811_RDCFG_RESERVED (1 << 1)

#define LTC6811_RDCVA_RESERVED (1 << 2)

#define LTC6811_RDCVB_RESERVED (1 << 2) | (1 << 1)

#define LTC6811_RDCVC_RESERVED (1 << 3)

#define LTC6811_RDCVD_RESERVED (1 << 3) | (1 << 1)

#define LTC6811_RDAUXA_RESERVED ((1 << 3) | (1 << 2))

#define LTC6811_RDAUXB_RESERVED ((1 << 3) | (1 << 2)) | (1 << 1)

#define LTC6811_RDSTATA_RESERVED (1 << 4)

#define LTC6811_RDSTATB_RESERVED (1 << 4) | (1 << 1)

#define LTC6811_ADCV_RESERVED ((1 << 9) | (1 << 6) | (1 << 5))

#define LTC6811_ADCOW_RESERVED ((1 << 3) | (1 << 5) | (1 << 9))

#define LTC6811_CVST_RESERVED ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 9))

#define LTC6811_ADAX_RESERVED (1 << 10) | (1 << 6) | (1 << 5)

#define LTC6811_CLRCELL_RESERVED (1 << 0) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_CLRAUX_RESERVED (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_CLRSTAT_RESERVED (1 << 0) | (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_PLADC_RESERVED (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_DIAGNC_RESERVED (1 << 0) | (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_WRCOMM_RESERVED (1 << 0) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_RDCOMM_RESERVED (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6811_STCOMM_RESERVED (1 << 0) | (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)

// command bits
// see Table 40 (p. 62)
#define LTC6811_GPIO1_PD_ON (0 << 3)
#define LTC6811_GPIO1_PD_OFF (1 << 3)
#define LTC6811_GPIO2_PD_ON (0 << 4)
#define LTC6811_GPIO2_PD_OFF (1 << 4)
#define LTC6811_GPIO3_PD_ON (0 << 5)
#define LTC6811_GPIO3_PD_OFF (1 << 5)
#define LTC6811_GPIO4_PD_ON (0 << 6)
#define LTC6811_GPIO4_PD_OFF (1 << 6)
#define LTC6811_GPIO5_PD_ON (0 << 7)
#define LTC6811_GPIO5_PD_OFF (1 << 7)

#define LTC6811_CNVT_CELL_ALL 0x00
#define LTC6811_CNVT_CELL_1_7 0x01
#define LTC6811_CNVT_CELL_2_8 0x02
#define LTC6811_CNVT_CELL_3_9 0x03
#define LTC6811_CNVT_CELL_4_10 0x04
#define LTC6811_CNVT_CELL_5_11 0x05
#define LTC6811_CNVT_CELL_6_12 0x06

#define LTC6811_ADCV_DISCHARGE_NOT_PERMITTED (0 << 4)
#define LTC6811_ADCV_DISCHARGE_PERMITTED (1 << 4)

#define LTC6811_ADCOPT (1 << 0)

#define LTC6811_SWTRD (1 << 1)

#define LTC6811_ADAX_GPIO1 0x01
#define LTC6811_ADAX_MODE_FAST (0 << 8) | (1 << 7)

#define LTC6811_ICOM_CSBM_LOW (1 << 3)
#define LTC6811_ICOM_CSBM_HIGH (1 << 3) | 1
#define LTC6811_ICOM_NO_TRANSMIT (1 << 3) | (1 << 2) | (1 << 1) | 1

#define LTC6811_FCOM_CSBM_LOW 0
#define LTC6811_FCOM_CSBM_HIGH (1 << 3) | 1
