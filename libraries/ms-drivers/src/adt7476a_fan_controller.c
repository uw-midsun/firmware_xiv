#include "adt7476a_fan_controller.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
#include "adt7476a_fan_controller_defs.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "soft_timer.h"

static int mpxe_initial_conditions;  // If 1, read init conditions from store

#ifdef MPXE
#include <stdlib.h>
#include "adt7476a.pb-c.h"
#include "store.h"
#include "stores.pb-c.h"

static MxAdt7476aStore s_store = MX_ADT7476A_STORE__INIT;

static void update_store(ProtobufCBinaryData msg_buf, ProtobufCBinaryData mask_buf) {
  MxAdt7476aStore *msg = mx_adt7476a_store__unpack(NULL, msg_buf.len, msg_buf.data);
  MxAdt7476aStore *mask = mx_adt7476a_store__unpack(NULL, mask_buf.len, mask_buf.data);

  for (AdtPwmPort i = 0; i < NUM_ADT_PWM_PORTS; i++) {
    // only update state if mask is set
    if (mask->status[i] != 0) {
      s_store.speed[i] = msg->speed[i];
      s_store.status[i] = msg->status[i];
    }
  }

  mx_adt7476a_store__free_unpacked(msg, NULL);
  mx_adt7476a_store__free_unpacked(mask, NULL);
  store_export(MX_STORE_TYPE__ADT7476A, &s_store, NULL);
}

static void prv_init_store(void) {
  store_config();
  StoreFuncs funcs = {
    (GetPackedSizeFunc)mx_adt7476a_store__get_packed_size,
    (PackFunc)mx_adt7476a_store__pack,
    (UnpackFunc)mx_adt7476a_store__unpack,
    (FreeUnpackedFunc)mx_adt7476a_store__free_unpacked,
    (UpdateStoreFunc)update_store,
  };
  s_store.n_speed = NUM_ADT_PWM_PORTS;
  s_store.speed = malloc(NUM_ADT_PWM_PORTS * sizeof(uint32_t));
  s_store.n_status = NUM_ADT_PWM_PORTS;
  s_store.status = malloc(NUM_ADT_PWM_PORTS * sizeof(protobuf_c_boolean));
  store_register(MX_STORE_TYPE__ADT7476A, funcs, &s_store, NULL);
}
#endif

// need to set interrupt once fan goes out of range
// PWM duty cycle is set from 0-100, in steps of 0.39 (0x00 - 0xFF)
// accepts number between 0-100, converts into into range of 0x00 - 0xFF
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed_percent, AdtPwmPort pwm_port,
                              uint8_t adt7476a_i2c_address) {
  // check for out of range conditions.
  if (speed_percent > 100) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  // determine which PWM output to change
  uint8_t real_speed = (speed_percent) ? (speed_percent / 0.39 - 1) : 0;
  if (pwm_port == ADT_PWM_PORT_1) {
    uint8_t adt7476a_speed_register[] = { ADT7476A_PWM_1, real_speed };

    status_ok_or_return(i2c_write(port, adt7476a_i2c_address, adt7476a_speed_register,
                                  SIZEOF_ARRAY(adt7476a_speed_register)));
#ifdef MPXE
    s_store.speed[0] = real_speed;
#endif

  } else if (pwm_port == ADT_PWM_PORT_2) {
    uint8_t adt7476a_speed_register[] = { ADT7476A_PWM_3, real_speed };

    status_ok_or_return(i2c_write(port, adt7476a_i2c_address, adt7476a_speed_register,
                                  SIZEOF_ARRAY(adt7476a_speed_register)));
#ifdef MPXE
    s_store.speed[1] = real_speed;
#endif

  } else if (pwm_port == ADT_PWM_PORT_3) {
    return STATUS_CODE_UNIMPLEMENTED;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }

#ifdef MPXE
  store_export(MX_STORE_TYPE__ADT7476A, &s_store, NULL);
#endif
  return STATUS_CODE_OK;
}

StatusCode adt7476a_get_status(I2CPort port, uint8_t adt7476a_i2c_read_address,
                               uint8_t *register_1_data, uint8_t *register_2_data) {
  // read interrupt status register
  StatusCode reg1 =
      (i2c_read_reg(port, adt7476a_i2c_read_address, ADT7476A_INTERRUPT_STATUS_REGISTER_1,
                    register_1_data, ADT7476A_REG_SIZE));
  StatusCode reg2 =
      (i2c_read_reg(port, adt7476a_i2c_read_address, ADT7476A_INTERRUPT_STATUS_REGISTER_2,
                    register_2_data, ADT7476A_REG_SIZE));
#ifdef MPXE
  s_store.status[0] = reg1 ? true : false;
  s_store.status[1] = reg2 ? true : false;

  store_export(MX_STORE_TYPE__ADT7476A, &s_store, NULL);
#endif
  return STATUS_CODE_OK;
}

StatusCode adt7476a_init(Adt7476aStorage *storage, Adt7476aSettings *settings) {
#ifdef MPXE
  prv_init_store();
  mpxe_initial_conditions = read_init_conditions();
#endif
  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Adt7476aStorage));

  storage->smbalert_pin = settings->smbalert_pin;
  storage->callback = settings->callback;  // called when tachometer goes out of range
  storage->callback_context = settings->callback_context;
  storage->i2c = settings->i2c;

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  uint8_t fan_config_data_register_1[] = { ADT7476A_FAN_MODE_REGISTER_1,
                                           ADT7476A_MANUAL_MODE_MASK };
  uint8_t fan_config_data_register_3[] = { ADT7476A_FAN_MODE_REGISTER_3,
                                           ADT7476A_MANUAL_MODE_MASK };
  uint8_t strt_config_data[] = { ADT7476A_CONFIG_REGISTER_1, ADT7476A_CONFIG_REG_1_MASK };
  uint8_t smbalert_config_data[] = { ADT7476A_CONFIG_REGISTER_3, ADT7476A_CONFIG_REG_3_MASK };

  // set STRT bit to on
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, strt_config_data,
                                SIZEOF_ARRAY(strt_config_data)));

  // configure pwm to manual mode
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, fan_config_data_register_1,
                                SIZEOF_ARRAY(fan_config_data_register_1)));
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, fan_config_data_register_3,
                                SIZEOF_ARRAY(fan_config_data_register_3)));

  // set pin 10 to SMBALERT rather than pwm output
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, smbalert_config_data,
                                SIZEOF_ARRAY(smbalert_config_data)));

  gpio_it_register_interrupt(&storage->smbalert_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             storage->callback, storage->callback_context);

#ifdef MPXE
  store_export(MX_STORE_TYPE__ADT7476A, &s_store, NULL);
#endif

  return STATUS_CODE_OK;
}
