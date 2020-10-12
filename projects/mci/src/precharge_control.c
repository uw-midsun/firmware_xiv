#include "precharge_control.h"

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "motor_controller.h"
#include "status.h"

static PrechargeControlStorage s_precharge_storage = { 0 };

PrechargeControlStorage *test_get_storage(void) {
  return &s_precharge_storage;
}

static void prv_check_state() {
  uint8_t a10 = 0, b0 = 0;
  GpioAddress a_a10 = { .port = GPIO_PORT_A, .pin = 10 };
  GpioAddress a_b0 = { .port = GPIO_PORT_B, .pin = 0 };
  gpio_get_state(&a_a10, &a10);
  gpio_get_state(&a_b0, &b0);
  printf("precharge state - A10: %d, B0: %d\n", a10, b0);
}

StatusCode prv_set_precharge_control(PrechargeControlStorage *storage, const GpioState state) {
  printf("%s\n", __func__);
  prv_check_state();
  gpio_set_state(&storage->precharge_control, state);
  printf("%s\n", __func__);
  prv_check_state();
  return STATUS_CODE_OK;
}

void prv_precharge_monitor(const GpioAddress *address, void *context) {
  PrechargeControlStorage *storage = context;
  printf("%s\n", __func__);
  prv_check_state();
  if (storage->state == MCI_PRECHARGE_DISCHARGED) {
    // inconsistent until second precharge result
    storage->state = MCI_PRECHARGE_INCONSISTENT;
  } else {
    // both pins are in sync
    storage->state = MCI_PRECHARGE_CHARGED;
    CAN_TRANSMIT_PRECHARGE_COMPLETED();
  }
}

StatusCode prv_discharge_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  PrechargeControlStorage *storage = context;
  storage->state = MCI_PRECHARGE_DISCHARGED;
  return prv_set_precharge_control(storage, GPIO_STATE_LOW);
}

StatusCode prv_precharge_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  printf("%s\n", __func__);
  prv_check_state();
  PrechargeControlStorage *storage = context;
  storage->state = MCI_PRECHARGE_CHARGED;
  // CAN_TRANSMIT_PRECHARGE_COMPLETED();
  return prv_set_precharge_control(storage, GPIO_STATE_HIGH);
}

PrechargeState get_precharge_state() {
  return s_precharge_storage.state;
}

void prv_populate_storage(PrechargeControlStorage *storage,
                          const PrechargeControlSettings *settings) {
  storage->state = MCI_PRECHARGE_DISCHARGED;
  storage->precharge_control = settings->precharge_control;
  storage->precharge_monitor = settings->precharge_monitor;
  storage->precharge_monitor2 = settings->precharge_monitor2;
  storage->initialized = true;
}

StatusCode precharge_control_init(const PrechargeControlSettings *settings) {
  if (s_precharge_storage.initialized) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  prv_populate_storage(&s_precharge_storage, settings);
  GpioSettings monitor_settings = { .direction = GPIO_DIR_IN,
                                    .state = GPIO_STATE_LOW,
                                    .alt_function = GPIO_ALTFN_NONE,
                                    .resistor = GPIO_RES_NONE };
  GpioSettings control_settings = monitor_settings;
  control_settings.direction = GPIO_DIR_OUT;
  InterruptSettings monitor_it_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                            .priority = INTERRUPT_PRIORITY_NORMAL };
  status_ok_or_return(gpio_init_pin(&s_precharge_storage.precharge_control, &control_settings));
  status_ok_or_return(gpio_init_pin(&s_precharge_storage.precharge_monitor, &monitor_settings));
  status_ok_or_return(gpio_init_pin(&s_precharge_storage.precharge_monitor2, &monitor_settings));
  status_ok_or_return(gpio_it_register_interrupt(&s_precharge_storage.precharge_monitor,
                                                 &monitor_it_settings, INTERRUPT_EDGE_RISING,
                                                 prv_precharge_monitor, &s_precharge_storage));
  status_ok_or_return(gpio_it_register_interrupt(&s_precharge_storage.precharge_monitor2,
                                                 &monitor_it_settings, INTERRUPT_EDGE_RISING,
                                                 prv_precharge_monitor, &s_precharge_storage));
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BEGIN_PRECHARGE, prv_precharge_rx,
                                              &s_precharge_storage));
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_DISCHARGE_PRECHARGE,
                                              prv_discharge_rx, &s_precharge_storage));
  return STATUS_CODE_OK;
}
