#pragma once

#define PLUTUS_CFG_AFE_SPI_PORT SPI_PORT_1
#define PLUTUS_CFG_AFE_SPI_BAUDRATE 750000
#define PLUTUS_CFG_AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }
#define PLUTUS_CFG_AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define PLUTUS_CFG_AFE_SPI_SCLK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define PLUTUS_CFG_AFE_SPI_CS \
  { .port = GPIO_PORT_A, .pin = 4 }

// Use normal mode
#define PLUTUS_CFG_AFE_MODE LTC_AFE_ADC_MODE_7KHZ

// Using all 12 cell inputs
#define PLUTUS_CFG_INPUT_BITSET_FULL 0xFFF
// Using 6 cell inputs split across the 2 muxes
#define PLUTUS_CFG_INPUT_BITSET_SPLIT (0x7 << 6 | 0x7)
// Using first 6 cell inputs
#define PLUTUS_CFG_INPUT_BITSET_FIRST 0x3F

#ifdef PLUTUS_CFG_DEBUG_PACK
#define PLUTUS_CFG_AFE_DEVICES_IN_CHAIN 4
#define PLUTUS_CFG_AFE_TOTAL_CELLS 48
// We're using 18 modules per box -> 2 AFEs each
// clang-format off
  #define PLUTUS_CFG_CELL_BITSET_ARR                                  \
    { PLUTUS_CFG_INPUT_BITSET_FULL, PLUTUS_CFG_INPUT_BITSET_FULL,    \
      PLUTUS_CFG_INPUT_BITSET_FULL, PLUTUS_CFG_INPUT_BITSET_FULL, }
  #define PLUTUS_CFG_AUX_BITSET_ARR                                   \
    { PLUTUS_CFG_INPUT_BITSET_FULL, PLUTUS_CFG_INPUT_BITSET_FULL,    \
      PLUTUS_CFG_INPUT_BITSET_FULL, PLUTUS_CFG_INPUT_BITSET_FULL, }
// clang-format on
#else
// number of devices in daisy chain (including master)
#define PLUTUS_CFG_AFE_DEVICES_IN_CHAIN 1
#define PLUTUS_CFG_AFE_TOTAL_CELLS 1
// We're using 18 modules per box -> 2 AFEs each
// clang-format off
  #define PLUTUS_CFG_CELL_BITSET_ARR                                  \
    { PLUTUS_CFG_INPUT_BITSET_FULL }
  #define PLUTUS_CFG_AUX_BITSET_ARR                                   \
    { PLUTUS_CFG_INPUT_BITSET_FULL }
// clang-format on
#endif
