#include "pca9539r_gpio_expander.h"
#ifdef MPXE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pca9539r.pb-c.h"
#include "store.h"
#include "stores.pb-c.h"

static MxPca9539rStore s_store = MX_PCA9539R_STORE__INIT;
#endif

// There's only 256 I2C addresses so it's ok to keep all the settings in memory
#define MAX_I2C_ADDRESSES 256

// Note: not necessary for x86 but kept to emulate stm32's uninitialized behaviour
static I2CPort s_i2c_port = NUM_I2C_PORTS;

static Pca9539rGpioSettings s_pin_settings[MAX_I2C_ADDRESSES][NUM_PCA9539R_GPIO_PINS];

#ifdef MPXE
static void prv_export() {
  for (uint16_t i = 0; i < NUM_PCA9539R_GPIO_PINS; i++) {
    // s_store.state[i] = s_pin_settings[i].state;
  }
  store_export(MX_STORE_TYPE__PCA9539A, &s_store, NULL);
}

static void update_store(ProtobufCBinaryData msg_buf, ProtobufCBinaryData mask_buf) {
  MxGpioStore *msg = mx_gpio_store__unpack(NULL, msg_buf.len, msg_buf.data);
  MxGpioStore *mask = mx_gpio_store__unpack(NULL, mask_buf.len, mask_buf.data);

  for (uint16_t i = 0; i < mask->n_state; i++) {
    // only update state if mask is set
    if (mask->state[i] != 0) {
      s_store.state[i] = msg->state[i];
      if (s_pin_settings[i].state != (uint8_t)msg->state[i]) {
        // s_pin_settings[i].state = msg->state[i];
        // // Note that interrupts are ID'd based on pin only, not port.
        // GpioAddress address = { .port = 0, .pin = i % 16 };
        // InterruptEdge edge;
        // if (status_ok(gpio_it_get_edge(&address, &edge))) {
        //   if ((msg->state[i] == 1 && edge == INTERRUPT_EDGE_RISING) ||
        //       (msg->state[i] == 0 && edge == INTERRUPT_EDGE_FALLING) ||
        //       edge == INTERRUPT_EDGE_RISING_FALLING) {
        //     gpio_it_trigger_interrupt(&address);
        //   }
        // }
      }
    }
  }

  mx_gpio_store__free_unpacked(msg, NULL);
  mx_gpio_store__free_unpacked(mask, NULL);
  prv_export();
}

static void prv_init_store(void) {
  store_config();
  StoreFuncs funcs = {
    (GetPackedSizeFunc)mx_pca9539r_store__get_packed_size,
    (PackFunc)mx_pca9539r_store__pack,
    (UnpackFunc)mx_pca9539r_store__unpack,
    (FreeUnpackedFunc)mx_pca9539r_store__free_unpacked,
    (UpdateStoreFunc)update_store,
  };
  s_store.n_state = MAX_I2C_ADDRESSES * NUM_PCA9539R_GPIO_PINS;
  s_store.state = malloc(MAX_I2C_ADDRESSES * NUM_PCA9539R_GPIO_PINS * sizeof(protobuf_c_boolean));
  store_register(MX_STORE_TYPE__PCA9539A, funcs, &s_store, NULL);
}
#endif

StatusCode pca9539r_gpio_init(const I2CPort i2c_port, const I2CAddress i2c_address) {
  s_i2c_port = i2c_port;

  // Set each pin to the default settings
  Pca9539rGpioSettings default_settings = {
    .direction = PCA9539R_GPIO_DIR_IN,
    .state = PCA9539R_GPIO_STATE_LOW,
  };
  for (Pca9539rPinAddress i = 0; i < NUM_PCA9539R_GPIO_PINS; i++) {
    s_pin_settings[i2c_address][i] = default_settings;
  }
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_init_pin(const Pca9539rGpioAddress *address,
                                  const Pca9539rGpioSettings *settings) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS || settings->direction >= NUM_PCA9539R_GPIO_DIRS ||
      settings->state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin] = *settings;
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_set_state(const Pca9539rGpioAddress *address,
                                   const Pca9539rGpioState state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS || state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin].state = state;
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_toggle_state(const Pca9539rGpioAddress *address) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (s_pin_settings[address->i2c_address][address->pin].state == PCA9539R_GPIO_STATE_HIGH) {
    s_pin_settings[address->i2c_address][address->pin].state = PCA9539R_GPIO_STATE_LOW;
  } else {
    s_pin_settings[address->i2c_address][address->pin].state = PCA9539R_GPIO_STATE_HIGH;
  }
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_get_state(const Pca9539rGpioAddress *address,
                                   Pca9539rGpioState *input_state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *input_state = s_pin_settings[address->i2c_address][address->pin].state;
  return STATUS_CODE_OK;
}
