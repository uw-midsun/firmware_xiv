#pragma once
// Standardized controller board ports and pins
// Defined as a GPIO Address

// CAN
#define CONTROLLER_BOARD_ADDR_DEBUG_CAN_RX \
  \
{                                       \
    GPIO_PORT_A, 11                        \
  }

#define CONTROLLER_BOARD_ADDR_DEBUG_CAN_TX \
  \
{                                       \
    GPIO_PORT_A, 12                        \
  }

// USART 1
#define CONTROLLER_BOARD_ADDR_DEBUG_USART1_TX \
  \
{                                          \
    GPIO_PORT_B, 6                            \
  }

#define CONTROLLER_BOARD_ADDR_DEBUG_USART1_RX \
  \
{                                          \
    GPIO_PORT_B, 7                            \
  }

// USART 2
#define CONTROLLER_BOARD_ADDR_USART2_TX \
  \
{                                    \
    GPIO_PORT_A, 2                      \
  }

#define CONTROLLER_BOARD_ADDR_USART2_RX \
  \
{                                    \
    GPIO_PORT_A, 3                      \
  }

// USART 3
#define CONTROLLER_BOARD_ADDR_USART3_TX \
  \
{                                    \
    GPIO_PORT_B, 10                     \
  }

#define CONTROLLER_BOARD_ADDR_USART3_RX \
  \
{                                    \
    GPIO_PORT_B, 11                     \
  }

// SPI 1
#define CONTROLLER_BOARD_ADDR_SPI1_NSS \
  \
{                                   \
    GPIO_PORT_A, 4                     \
  }

#define CONTROLLER_BOARD_ADDR_SPI1_SCK \
  \
{                                   \
    GPIO_PORT_A, 5                     \
  }

#define CONTROLLER_BOARD_ADDR_SPI1_MISO \
  \
{                                    \
    GPIO_PORT_A, 6                      \
  }

#define CONTROLLER_BOARD_ADDR_SPI1_MOSI \
  \
{                                    \
    GPIO_PORT_A, 7                      \
  }

// SPI 2
#define CONTROLLER_BOARD_ADDR_SPI2_NSS \
  \
{                                   \
    GPIO_PORT_B, 12                    \
  }

#define CONTROLLER_BOARD_ADDR_SPI2_SCK \
  \
{                                   \
    GPIO_PORT_B, 13                    \
  }

#define CONTROLLER_BOARD_ADDR_SPI2_MISO \
  \
{                                    \
    GPIO_PORT_B, 14                     \
  }

#define CONTROLLER_BOARD_ADDR_SPI2_MOSI \
  \
{                                    \
    GPIO_PORT_B, 15                     \
  }

// I2C 1
#define CONTROLLER_BOARD_ADDR_I2C1_SCL \
  \
{                                   \
    GPIO_PORT_B, 8                     \
  }

#define CONTROLLER_BOARD_ADDR_I2C1_SDA \
  \
{                                   \
    GPIO_PORT_B, 9                     \
  }

// I2C 2
#define CONTROLLER_BOARD_ADDR_I2C2_SCL \
  \
{                                   \
    GPIO_PORT_B, 10                    \
  }

#define CONTROLLER_BOARD_ADDR_I2C2_SDA \
  \
{                                   \
    GPIO_PORT_B, 11                    \
  }

// Onboard LEDs
#define CONTROLLER_BOARD_ADDR_LED_BLUE \
  \
{                                   \
    GPIO_PORT_A, 15                    \
  }

#define CONTROLLER_BOARD_ADDR_LED_RED \
  \
{                                  \
    GPIO_PORT_B, 3                    \
  }

#define CONTROLLER_BOARD_ADDR_LED_YELLOW \
  \
{                                     \
    GPIO_PORT_B, 4                       \
  }

#define CONTROLLER_BOARD_ADDR_LED_GREEN \
  \
{                                    \
    GPIO_PORT_B, 5                      \
  }

// PWM
#define CONTROLLER_BOARD_ADDR_PWM_TIM1 \
  \
{                                   \
    GPIO_PORT_A, 8                     \
  }

#define CONTROLLER_BOARD_ADDR_PWM_TIM3 \
  \
{                                   \
    GPIO_PORT_B, 0                     \
  }

#define CONTROLLER_BOARD_ADDR_PWM_TIM14 \
  \
{                                    \
    GPIO_PORT_A, 4                      \
  }

// SWD
#define CONTROLLER_BOARD_ADDR_SWDIO \
  \
{                                \
    GPIO_PORT_A, 13                 \
  }

#define CONTROLLER_BOARD_ADDR_SWCLK \
  \
{                                \
    GPIO_PORT_A, 14                 \
  }
