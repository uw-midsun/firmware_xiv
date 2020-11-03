#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>

#include "log.h"
#include "status.h"

#ifdef MPXE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gpio.pb-c.h"
#include "gpio_it.h"
#include "store.h"
#include "stores.pb-c.h"

static MxGpioStore s_store = MX_GPIO_STORE__INIT;
#endif

static GpioSettings s_pin_settings[GPIO_TOTAL_PINS];
static uint8_t s_gpio_pin_input_value[GPIO_TOTAL_PINS];

static uint32_t prv_get_index(const GpioAddress *address) {
  return address->port * (uint32_t)GPIO_PINS_PER_PORT + address->pin;
}

#ifdef MPXE
static void prv_export() {
  for (uint16_t i = 0; i < GPIO_TOTAL_PINS; i++) {
    s_store.state[i] = s_pin_settings[i].state;
  }
  store_export(MX_STORE_TYPE__GPIO, &s_store, NULL);
}

static void update_store(ProtobufCBinaryData msg_buf, ProtobufCBinaryData mask_buf) {
  MxGpioStore *msg = mx_gpio_store__unpack(NULL, msg_buf.len, msg_buf.data);
  MxGpioStore *mask = mx_gpio_store__unpack(NULL, mask_buf.len, mask_buf.data);

  for (uint16_t i = 0; i < mask->n_state; i++) {
    // only update state if mask is set
    if (mask->state[i] != 0) {
      s_store.state[i] = msg->state[i];
      if (s_pin_settings[i].state != (uint8_t)msg->state[i]) {
        s_pin_settings[i].state = msg->state[i];
        // Note that interrupts are ID'd based on pin only, not port.
        GpioAddress address = { .port = 0, .pin = i % 16 };
        InterruptEdge edge;
        if (status_ok(gpio_it_get_edge(&address, &edge))) {
          if ((msg->state[i] == 1 && edge == INTERRUPT_EDGE_RISING) ||
              (msg->state[i] == 0 && edge == INTERRUPT_EDGE_FALLING) ||
              edge == INTERRUPT_EDGE_RISING_FALLING) {
            gpio_it_trigger_interrupt(&address);
          }
        }
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
    (GetPackedSizeFunc)mx_gpio_store__get_packed_size,
    (PackFunc)mx_gpio_store__pack,
    (UnpackFunc)mx_gpio_store__unpack,
    (FreeUnpackedFunc)mx_gpio_store__free_unpacked,
    (UpdateStoreFunc)update_store,
  };
  s_store.n_state = GPIO_TOTAL_PINS;
  s_store.state = malloc(GPIO_TOTAL_PINS * sizeof(protobuf_c_boolean));
  store_register(MX_STORE_TYPE__GPIO, funcs, &s_store, NULL);
}
#endif

StatusCode gpio_init(void) {
#ifdef MPXE
  prv_init_store();
#endif
  GpioSettings default_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (uint32_t i = 0; i < GPIO_TOTAL_PINS; i++) {
    s_pin_settings[i] = default_settings;
    s_gpio_pin_input_value[i] = 0;
  }
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GpioAddress *address, const GpioSettings *settings) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      settings->direction >= NUM_GPIO_DIRS || settings->state >= NUM_GPIO_STATES ||
      settings->resistor >= NUM_GPIO_RESES || settings->alt_function >= NUM_GPIO_ALTFNS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[prv_get_index(address)] = *settings;
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode gpio_set_state(const GpioAddress *address, GpioState state) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      state >= NUM_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[prv_get_index(address)].state = state;
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GpioAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);
  if (s_pin_settings[index].state == GPIO_STATE_LOW) {
    s_pin_settings[index].state = GPIO_STATE_HIGH;
  } else {
    s_pin_settings[index].state = GPIO_STATE_LOW;
  }
#ifdef MPXE
  prv_export();
#endif
  return STATUS_CODE_OK;
}

StatusCode gpio_get_state(const GpioAddress *address, GpioState *state) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);

  // Behave how hardware does when the direction is set to out.
  if (s_pin_settings[index].direction != GPIO_DIR_IN) {
    *state = s_pin_settings[index].state;
  } else {
    *state = s_gpio_pin_input_value[index];
  }
  return STATUS_CODE_OK;
}
