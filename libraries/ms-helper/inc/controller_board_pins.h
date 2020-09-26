#pragma once
// Standardized controller board ports and pins
// Defined as a GPIO Address

// CAN
#define CONTROLLER_BOARD_ADDR_DEBUG_CAN_RX \
  \
{                                       \
    .port = GPIO_PORT_A, .pin = 11         \
  }

#define CONTROLLER_BOARD_ADDR_DEBUG_CAN_TX \
  \
{                                       \
    .port = GPIO_PORT_A, .pin = 12         \
  }

// USART 1
#define CONTROLLER_BOARD_ADDR_DEBUG_USART1_TX \
  \
{                                          \
    .port = GPIO_PORT_B, .pin = 6             \
  }

#define CONTROLLER_BOARD_ADDR_DEBUG_USART1_RX \
  \
{                                          \
    .port = GPIO_PORT_B, .pin = 7             \
  }

// USART 2
#define CONTROLLER_BOARD_ADDR_USART2_TX \
  \
{                                    \
    .port = GPIO_PORT_A, .pin = 2       \
  }

#define CONTROLLER_BOARD_ADDR_USART2_RX \
  \
{                                    \
    .port = GPIO_PORT_A, .pin = 3       \
  }

// USART 3
#define CONTROLLER_BOARD_ADDR_USART3_TX \
  \
{                                    \
    .port = GPIO_PORT_B, .pin = 10      \
  }

#define CONTROLLER_BOARD_ADDR_USART3_RX \
  \
{                                    \
    .port = GPIO_PORT_B, .pin = 11      \
  }

// SPI 1
#define CONTROLLER_BOARD_ADDR_SPI1_NSS \
  \
{                                   \
    .port = GPIO_PORT_A, .pin = 4      \
  }

#define CONTROLLER_BOARD_ADDR_SPI1_SCK \
  \
{                                   \
    .port = GPIO_PORT_A, .pin = 5      \
  }

#define CONTROLLER_BOARD_ADDR_SPI1_MISO \
  \
{                                    \
    .port = GPIO_PORT_A, .pin = 6       \
  }

#define CONTROLLER_BOARD_ADDR_SPI1_MOSI \
  \
{                                    \
    .port = GPIO_PORT_A, .pin = 7       \
  }

// SPI 2
#define CONTROLLER_BOARD_ADDR_SPI2_NSS \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 12     \
  }

#define CONTROLLER_BOARD_ADDR_SPI2_SCK \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 13     \
  }

#define CONTROLLER_BOARD_ADDR_SPI2_MISO \
  \
{                                    \
    .port = GPIO_PORT_B, .pin = 14      \
  }

#define CONTROLLER_BOARD_ADDR_SPI2_MOSI \
  \
{                                    \
    .port = GPIO_PORT_B, .pin = 15      \
  }

// I2C 1
#define CONTROLLER_BOARD_ADDR_I2C1_SCL \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 8      \
  }

#define CONTROLLER_BOARD_ADDR_I2C1_SDA \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 9      \
  }

// I2C 2
#define CONTROLLER_BOARD_ADDR_I2C2_SCL \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 10     \
  }

#define CONTROLLER_BOARD_ADDR_I2C2_SDA \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 11     \
  }

// Onboard LEDs
#define CONTROLLER_BOARD_ADDR_LED_BLUE \
  \
{                                   \
    .port = GPIO_PORT_A, .pin = 15     \
  }

#define CONTROLLER_BOARD_ADDR_LED_RED \
  \
{                                  \
    .port = GPIO_PORT_B, .pin = 3     \
  }

#define CONTROLLER_BOARD_ADDR_LED_YELLOW \
  \
{                                     \
    .port = GPIO_PORT_B, .pin = 4        \
  }

#define CONTROLLER_BOARD_ADDR_LED_GREEN \
  \
{                                    \
    .port = GPIO_PORT_B, .pin = 5       \
  }

// PWM
#define CONTROLLER_BOARD_ADDR_PWM_TIM1 \
  \
{                                   \
    .port = GPIO_PORT_A, .pin = 8      \
  }

#define CONTROLLER_BOARD_ADDR_PWM_TIM3 \
  \
{                                   \
    .port = GPIO_PORT_B, .pin = 0      \
  }

#define CONTROLLER_BOARD_ADDR_PWM_TIM14 \
  \
{                                    \
    .port = GPIO_PORT_A, .pin = 4       \
  }

// SWD
#define CONTROLLER_BOARD_ADDR_SWDIO \
  \
{                                \
    .port = GPIO_PORT_A, .pin = 13  \
  }

#define CONTROLLER_BOARD_ADDR_SWCLK \
  \
{                                \
    .port = GPIO_PORT_A, .pin = 14  \
  }
