#pragma once
// Generic blocking SPI driver
// Requires GPIO to be initialized.
#include <stddef.h>
#include "gpio.h"
#include "spi_mcu.h"
#include "status.h"

typedef enum {
  SPI_MODE_0 = 0,  // CPOL: 0 CPHA: 0
  SPI_MODE_1,      // CPOL: 0 CPHA: 1
  SPI_MODE_2,      // CPOL: 1 CPHA: 0
  SPI_MODE_3,      // CPOL: 1 CPHA: 1
  NUM_SPI_MODES,
} SpiMode;

typedef struct {
  uint32_t baudrate;
  SpiMode mode;

  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;
  GpioAddress cs;
} SpiSettings;

// Note that our prescalers on STM32 must be a power of 2, so the actual
// baudrate may not be exactly as requested. Please verify that the actual
// baudrate is within bounds.
StatusCode spi_init(SpiPort spi, const SpiSettings *settings);

// This method will send |tx_len| bytes from |tx_data| to the spi port |spi|. It
// will not change the CS line state. The response bytes will be discarded.
StatusCode spi_tx(SpiPort spi, uint8_t *tx_data, size_t tx_len);

// This method will receive |rx_len| bytes and place it into |rx_data| from the
// spi port |spi|. It will not change the CS line state. In order to receive
// data this method will send the byte specified by the |placeholder| parameter.
StatusCode spi_rx(SpiPort spi, uint8_t *rx_data, size_t rx_len, uint8_t placeholder);

// This method will set the state of the CS line for a given spi port
StatusCode spi_cs_set_state(SpiPort spi, GpioState state);

// Gets the CS state of a given spi port and assigns it to the state that is
// passed in.
StatusCode spi_cs_get_state(SpiPort spi, GpioState *input_state);

// This method is a wrapper for |spi_tx| and |spi_rx|. First it will call
// |spi_tx|, then |spi_rx|. Before tx and rx, CS will be pulled low. After,
// CS will be pulled high. As a rule of thumb, this method should be used in
// simple transactions where at most one TX is required, followed by at most one
// RX.
StatusCode spi_exchange(SpiPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                        size_t rx_len);
