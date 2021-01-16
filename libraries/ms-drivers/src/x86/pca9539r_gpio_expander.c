#include "pca9539r_gpio_expander.h"

// There's only 256 I2C addresses so it's ok to keep all the settings in memory
#define MAX_I2C_ADDRESSES 256

// Note: not necessary for x86 but kept to emulate stm32's uninitialized behaviour
static I2CPort s_i2c_port = NUM_I2C_PORTS;

static Pca9539rGpioSettings s_pin_settings[MAX_I2C_ADDRESSES][NUM_PCA9539R_GPIO_PINS];

#ifdef MPXE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"

#include "pca9539r.pb-c.h"
#include "store.h"
#include "stores.pb-c.h"

static MxPca9539rStore s_stores[MAX_I2C_ADDRESSES];

#define PCA9539_I2C_ADDRESS 0x74  // PCA9539 address used in the smoke test

static void prv_export(void *key) {
  int k = (intptr_t)(key);
  for (uint16_t j = 0; j < NUM_PCA9539R_GPIO_PINS; j++) {
    s_stores[k].state[j] = s_pin_settings[k][j].state;
  }

  store_export(MX_STORE_TYPE__PCA9539R, &s_stores[k], key);
}

static void update_store(ProtobufCBinaryData msg_buf, ProtobufCBinaryData mask_buf, void *key) {
  MxPca9539rStore *msg  = mx_pca9539r_store__unpack(NULL, msg_buf.len, msg_buf.data);
  MxPca9539rStore *mask = mx_pca9539r_store__unpack(NULL, mask_buf.len, mask_buf.data);

  int k = (intptr_t)(key);
  for (uint16_t i = 0; i < mask->n_state; i++) {
    // only update state if mask is set
    if (mask->state[i] != 0) {
      s_stores[k].state[i] = msg->state[i];
      if (s_pin_settings[k][i].state != (uint)msg->state[i]) {
        s_pin_settings[k][i].state = msg->state[i];
      }
    }
  }

  mx_pca9539r_store__free_unpacked(msg, NULL);
  mx_pca9539r_store__free_unpacked(mask, NULL);
  prv_export(key);
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
  for (int i = 0; i < MAX_I2C_ADDRESSES; ++i) {
    s_stores[i] = (MxPca9539rStore)MX_PCA9539R_STORE__INIT;
    s_stores[i].n_state = NUM_PCA9539R_GPIO_PINS;
    s_stores[i].state = malloc(NUM_PCA9539R_GPIO_PINS * sizeof(protobuf_c_boolean));
    store_register(MX_STORE_TYPE__PCA9539R, funcs, &s_stores[i], (void *)(intptr_t)i);
  }
}
#endif

StatusCode pca9539r_gpio_init(const I2CPort i2c_port, const I2CAddress i2c_address) {
#ifdef MPXE
  prv_init_store();
#endif
  s_i2c_port = i2c_port;

  // Set each pin to the default settings
  Pca9539rGpioSettings default_settings = {
    .direction = PCA9539R_GPIO_DIR_IN,
    .state = PCA9539R_GPIO_STATE_LOW,
  };
  for (Pca9539rPinAddress i = 0; i < NUM_PCA9539R_GPIO_PINS; i++) {
    s_pin_settings[i2c_address][i] = default_settings;
  }
#ifdef MPXE
  prv_export((void *)(intptr_t)i2c_address);
#endif
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
#ifdef MPXE
  prv_export((void *)(intptr_t)address->i2c_address);
#endif
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
#ifdef MPXE
  prv_export((void *)(intptr_t)address->i2c_address);
#endif
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
#ifdef MPXE
  prv_export((void *)(intptr_t)address->i2c_address);
#endif
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
