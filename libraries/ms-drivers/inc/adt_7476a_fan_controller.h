#pragma once

// Requires interrupts, soft timers, GPIO, and GPIO interrupts to be initialized

typedef void (*Adt7476ADataCallback)(void *context);

typedef struct {
  GpioAddress smbalert_pin;
  uint32_t interval_ms;
  Adt7476ADataCallback callback;  // set to NULL for no callback
  void *callback_context;
  I2CPort i2c;
  I2CAddress i2c_read_addr;
  I2CAddress i2c_write_addr;
  I2CSettings i2c_settings;
} Adt7476aSettings;

typedef struct {
  GpioAddress smbalert_pin;
  uint32_t interval_ms;
  SoftTimerId timer_id;
  Adt7476ADataCallback callback;
  void *callback_context;
  I2CPort i2c;
} Adt7476aStorage;

// 2 pwm outputs, each controlling 2 fans
typedef enum { ADT_FAN_GROUP_1, ADT_FAN_GROUP_2, NUM_FANS } FAN_GROUP;

// Initialize the Adt7476a with the given settings; the select pin is an STM32 GPIO pin.
StatusCode adt7476a_init(Adt7476aStorage *storage, Adt7476aSettings *settings);

// Translate and write the new speed
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed, FAN_GROUP fan_group,
                              uint8_t adt_7476a_i2c_addr);

StatusCode get_status(I2CPort port, uint8_t adt_7476a_i2c_addr, uint8_t *register_1_data,
                      uint8_t *register_2_data);
