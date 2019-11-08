// In the case of a timeout, we attempt to recover by manually clocking SCL. The
// idea is that if the bus is locked up, a slave is likely holding SDA low in
// the middle of a transaction. By clocking SCL, we hopefully complete the
// slave's transaction and transition it into an idle state for the next valid
// transaction.
#include "i2c.h"
#include <stdbool.h>
#include "critical_section.h"
#include "log.h"
#include "stm32f0xx.h"

// Arbitrary timeout
#define I2C_TIMEOUT 100000
#define I2C_TIMEOUT_WHILE_FLAG(i2c_port, flag, status)                      \
  do {                                                                      \
    uint32_t timeout = (I2C_TIMEOUT);                                       \
    while (I2C_GetFlagStatus(s_port[i2c_port].base, flag) == status) {      \
      timeout--;                                                            \
      if (timeout == 0) {                                                   \
        LOG_DEBUG("Timeout: %lu waiting for %d to change\n", flag, status); \
        prv_recover_lockup(i2c_port);                                       \
        return status_code(STATUS_CODE_TIMEOUT);                            \
      }                                                                     \
    }                                                                       \
  } while (0)

#define I2C_STOP(i2c_port)                                   \
  do {                                                       \
    I2C_TIMEOUT_WHILE_FLAG(i2c_port, I2C_FLAG_STOPF, RESET); \
    I2C_ClearFlag(s_port[i2c_port].base, I2C_FLAG_STOPF);    \
  } while (0)

typedef struct {
  uint32_t periph;
  I2C_TypeDef *base;
  I2CSettings settings;
} I2CPortData;

static I2CPortData s_port[NUM_I2C_PORTS] = {
  [I2C_PORT_1] = { .periph = RCC_APB1Periph_I2C1, .base = I2C1 },
  [I2C_PORT_2] = { .periph = RCC_APB1Periph_I2C2, .base = I2C2 },
};

// Generated using the I2C timing configuration tool (STSW-STM32126)
static const uint32_t s_i2c_timing[] = {
  [I2C_SPEED_STANDARD] = 0x10805E89,  // 100 kHz
  [I2C_SPEED_FAST] = 0x00901850,      // 400 kHz
};

static void prv_recover_lockup(I2CPort port) {
  I2CSettings *settings = &s_port[port].settings;

  GpioSettings scl_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_NONE,
    .state = GPIO_STATE_LOW,
  };
  // Manually clock SCL
  gpio_init_pin(&settings->scl, &scl_settings);
  for (size_t i = 0; i < 16; i++) {
    gpio_toggle_state(&settings->scl);
  }

  scl_settings.direction = GPIO_DIR_OUT_OD;
  scl_settings.alt_function = GPIO_ALTFN_1;
  gpio_init_pin(&settings->scl, &scl_settings);

  // Reset I2C
  I2C_SoftwareResetCmd(s_port[port].base);
}

static StatusCode prv_transfer(I2CPort port, uint8_t addr, bool read, uint8_t *data, size_t len,
                               uint32_t end_mode) {
  I2C_TypeDef *i2c = s_port[port].base;

  I2C_TransferHandling(i2c, addr << 1, len, end_mode,
                       read ? I2C_Generate_Start_Read : I2C_Generate_Start_Write);

  if (read) {
    for (size_t i = 0; i < len; i++) {
      I2C_TIMEOUT_WHILE_FLAG(port, I2C_FLAG_RXNE, RESET);
      data[i] = I2C_ReceiveData(i2c);
    }
  } else {
    for (size_t i = 0; i < len; i++) {
      I2C_TIMEOUT_WHILE_FLAG(port, I2C_FLAG_TXIS, RESET);
      I2C_SendData(i2c, data[i]);
    }
    if (end_mode == I2C_SoftEnd_Mode) {
      I2C_TIMEOUT_WHILE_FLAG(port, I2C_FLAG_TC, RESET);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  } else if (settings->speed >= NUM_I2C_SPEEDS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C speed.");
  }

  s_port[i2c].settings = *settings;

  RCC_APB1PeriphClockCmd(s_port[i2c].periph, ENABLE);
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

  GpioSettings io_settings = {
    .direction = GPIO_DIR_OUT_OD,  //
    .alt_function = GPIO_ALTFN_1,  //
  };
  gpio_init_pin(&settings->scl, &io_settings);
  gpio_init_pin(&settings->sda, &io_settings);

  I2C_InitTypeDef i2c_init;
  I2C_StructInit(&i2c_init);
  i2c_init.I2C_Mode = I2C_Mode_I2C;
  i2c_init.I2C_Ack = I2C_Ack_Enable;
  i2c_init.I2C_Timing = s_i2c_timing[settings->speed];

  I2C_Init(s_port[i2c].base, &i2c_init);

  I2C_Cmd(s_port[i2c].base, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  CRITICAL_SECTION_AUTOEND;

  I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_BUSY, SET);

  status_ok_or_return(prv_transfer(i2c, addr, true, rx_data, rx_len, I2C_AutoEnd_Mode));

  I2C_STOP(i2c);

  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  CRITICAL_SECTION_AUTOEND;

  I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_BUSY, SET);

  status_ok_or_return(prv_transfer(i2c, addr, false, tx_data, tx_len, I2C_AutoEnd_Mode));

  I2C_STOP(i2c);

  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data,
                        size_t rx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  CRITICAL_SECTION_AUTOEND;

  I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_BUSY, SET);

  status_ok_or_return(prv_transfer(i2c, addr, false, &reg, sizeof(reg), I2C_SoftEnd_Mode));
  status_ok_or_return(prv_transfer(i2c, addr, true, rx_data, rx_len, I2C_AutoEnd_Mode));

  I2C_STOP(i2c);

  return STATUS_CODE_OK;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data,
                         size_t tx_len) {
  if (i2c >= NUM_I2C_PORTS) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid I2C port.");
  }
  CRITICAL_SECTION_AUTOEND;

  I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_BUSY, SET);

  status_ok_or_return(prv_transfer(i2c, addr, false, &reg, sizeof(reg), I2C_SoftEnd_Mode));
  status_ok_or_return(prv_transfer(i2c, addr, false, tx_data, tx_len, I2C_AutoEnd_Mode));

  I2C_STOP(i2c);

  return STATUS_CODE_OK;
}
