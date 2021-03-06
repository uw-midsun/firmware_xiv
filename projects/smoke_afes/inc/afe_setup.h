#pragma once

// Device config (don't change)
#define AFE_MODE LTC_AFE_ADC_MODE_7KHZ
#define AFE_SPI_PORT SPI_PORT_1
#define AFE_SPI_BAUDRATE 750000
#define AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }
#define AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define AFE_SPI_SCLK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define AFE_SPI_CS \
  { .port = GPIO_PORT_A, .pin = 4 }
typedef enum {
  AFE_TRIGGER_CELL_CONV_EVENT = 0,
  AFE_CELL_CONV_COMPLETE_EVENT,
  AFE_TRIGGER_AUX_CONV_EVENT,
  AFE_AUX_CONV_COMPLETE_EVENT,
  AFE_CALLBACK_RUN_EVENT,
  AFE_FAULT_EVENT,
  NUM_AFE_EVENTS,
} AfeEvents;
