#include "mcp23008_gpio_expander.h"

// There's only 256 I2C addresses so it's ok to keep all the settings in memory.
#define MAX_I2C_ADDRESSES 256

// Note: not necessary for x86 but kept to emulate stm32's uninitialized behaviour
static I2CPort s_i2c_port = NUM_I2C_PORTS;

static Mcp23008GpioSettings s_pin_settings[MAX_I2C_ADDRESSES][NUM_MCP23008_GPIO_PINS];

#include "log.h"

#ifdef MPXE
#include <stdlib.h>
#include "mcp23008.pb-c.h"
#include "store.h"
#include "stores.pb-c.h"

// MPXE version of mcp23008 only set up to handle one I2CAddress
static I2CAddress mpxe_address;

static MxMcp23008Store s_store = MX_MCP23008_STORE__INIT;

static void update_store(ProtobufCBinaryData msg_buf, ProtobufCBinaryData mask_buf) {
  MxMcp23008Store *msg = mx_mcp23008_store__unpack(NULL, msg_buf.len, msg_buf.data);
  MxMcp23008Store *mask = mx_mcp23008_store__unpack(NULL, mask_buf.len, mask_buf.data);
  for (uint8_t i = 0; i < NUM_MCP23008_GPIO_PINS; i++) {
    // only update state if mask is set
    if (mask->state[i] != 0) {
      s_store.state[i] = msg->state[i];
      if (s_pin_settings[mpxe_address][i].state != (uint8_t)msg->state[i]) {
        s_pin_settings[mpxe_address][i].state = (uint8_t)msg->state[i];
      }
    }
  }

  mx_mcp23008_store__free_unpacked(msg, NULL);
  mx_mcp23008_store__free_unpacked(mask, NULL);
  store_export(MX_STORE_TYPE__MCP23008, &s_store, NULL);
}

static void prv_init_store(void) {
  store_config();
  StoreFuncs funcs = {
    (GetPackedSizeFunc)mx_mcp23008_store__get_packed_size,
    (PackFunc)mx_mcp23008_store__pack,
    (UnpackFunc)mx_mcp23008_store__unpack,
    (FreeUnpackedFunc)mx_mcp23008_store__free_unpacked,
    (UpdateStoreFunc)update_store,
  };
  s_store.n_state = NUM_MCP23008_GPIO_PINS;
  s_store.state = malloc(NUM_MCP23008_GPIO_PINS * sizeof(protobuf_c_boolean));
  LOG_DEBUG("REGISTERING STORE\n");
  store_register(MX_STORE_TYPE__MCP23008, funcs, &s_store, NULL);
}

static void prv_export() {
  for (uint16_t i = 0; i < NUM_MCP23008_GPIO_PINS; i++) {
    s_store.state[i] = s_pin_settings[mpxe_address][i].state;
  }
  store_export(MX_STORE_TYPE__MCP23008, &s_store, NULL);
}
#endif

StatusCode mcp23008_gpio_init(const I2CPort i2c_port, const I2CAddress i2c_address) {
  s_i2c_port = i2c_port;
  // Set each pin to the default settings
  Mcp23008GpioSettings default_settings = {
    .direction = MCP23008_GPIO_DIR_IN,
    .state = MCP23008_GPIO_STATE_LOW,
  };
  for (Mcp23008PinAddress i = 0; i < NUM_MCP23008_GPIO_PINS; i++) {
    s_pin_settings[i2c_address][i] = default_settings;
  }

#ifdef MPXE
  mpxe_address = i2c_address;
  prv_init_store();
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_init_pin(const Mcp23008GpioAddress *address,
                                  const Mcp23008GpioSettings *settings) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_MCP23008_GPIO_PINS || settings->direction >= NUM_MCP23008_GPIO_DIRS ||
      settings->state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin] = *settings;
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_set_state(const Mcp23008GpioAddress *address,
                                   const Mcp23008GpioState state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_MCP23008_GPIO_PINS || state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin].state = state;
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_toggle_state(const Mcp23008GpioAddress *address) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (s_pin_settings[address->i2c_address][address->pin].state == MCP23008_GPIO_STATE_HIGH) {
    s_pin_settings[address->i2c_address][address->pin].state = MCP23008_GPIO_STATE_LOW;
  } else {
    s_pin_settings[address->i2c_address][address->pin].state = MCP23008_GPIO_STATE_HIGH;
  }
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_get_state(const Mcp23008GpioAddress *address,
                                   Mcp23008GpioState *input_state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *input_state = s_pin_settings[address->i2c_address][address->pin].state;
  return STATUS_CODE_OK;
}
