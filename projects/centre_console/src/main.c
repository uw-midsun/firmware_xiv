#include "brake_light_control.h"
#include "button_press.h"
#include "can.h"
#include "can_msg_defs.h"
#include "centre_console_events.h"
#include "charging_manager.h"
#include "controller_board_pins.h"
#include "delay.h"
#include "drive_fsm.h"
#include "event_queue.h"
#include "fault_monitor.h"
#include "gpio.h"
#include "gpio_it.h"
#include "hazard_tx.h"
#include "interrupt.h"
#include "led_manager.h"
#include "log.h"
#include "main_event_generator.h"
#include "pedal_monitor.h"
#include "power_aux_sequence.h"
#include "power_fsm.h"
#include "power_main_sequence.h"
#include "power_off_sequence.h"
#include "race_switch.h"
#include "regen_braking_toggle.h"
#include "soft_timer.h"
#include "speed_monitor.h"
#include "wait.h"

#define SPEED_MONITOR_WATCHDOG_TIMEOUT (1000 * 3)  // 3 seconds
#define FAULT_MONITOR_TIMEOUT (1000 * 3)

static CanStorage s_can_storage;

void prv_set_up_can(void) {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
  can_init(&s_can_storage, &can_settings);
}

static PowerAuxSequenceFsmStorage s_aux_sequence_storage = { 0 };
static PowerMainSequenceFsmStorage s_main_sequence_storage = { 0 };
static PowerOffSequenceStorage s_off_sequence_storage = { 0 };
static PowerFsmStorage s_power_fsm_storage = { 0 };
static DriveFsmStorage s_drive_fsm_storage = { 0 };
static RaceSwitchFsmStorage s_race_switch_fsm_storage = { 0 };

void prv_init_fsms() {
  power_main_sequence_init(&s_main_sequence_storage);
  power_aux_sequence_init(&s_aux_sequence_storage);
  power_off_sequence_init(&s_off_sequence_storage);
  power_fsm_init(&s_power_fsm_storage);
  drive_fsm_init(&s_drive_fsm_storage);
  race_switch_fsm_init(&s_race_switch_fsm_storage);
}

static MainEventGeneratorStorage s_main_event_generator = { 0 };

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  prv_set_up_can();

  pedal_monitor_init();
  button_press_init();

  I2CSettings i2c_settings = {
    .scl = CONTROLLER_BOARD_ADDR_I2C2_SCL,
    .sda = CONTROLLER_BOARD_ADDR_I2C2_SDA,
    .speed = I2C_SPEED_FAST,
  };
  i2c_init(MCP23008_I2C_PORT, &i2c_settings);
  led_manager_init();

  prv_init_fsms();

  init_charging_manager(&s_drive_fsm_storage.current_state);
  speed_monitor_init(SPEED_MONITOR_WATCHDOG_TIMEOUT);
  fault_monitor_init(FAULT_MONITOR_TIMEOUT);

  MainEventGeneratorResources resources = { .power_fsm = &s_power_fsm_storage,
                                            .drive_fsm = &s_drive_fsm_storage };

  main_event_generator_init(&s_main_event_generator, &resources);

  // turn on enable pin for raspberry pi
  GpioAddress raspi_addr = { .port = GPIO_PORT_B, .pin = 12 };
  GpioSettings raspi_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  gpio_init_pin(&raspi_addr, &raspi_settings);

  LOG_DEBUG("Hello from Centre Console!\n");

  while (true) {
    Event e = { 0 };
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      power_main_sequence_fsm_process_event(&s_main_sequence_storage, &e);
      power_aux_sequence_process_event(&s_aux_sequence_storage, &e);
      power_off_sequence_process_event(&s_off_sequence_storage, &e);
      power_fsm_process_event(&s_power_fsm_storage, &e);
      drive_fsm_process_event(&s_drive_fsm_storage, &e);
      main_event_generator_process_event(&s_main_event_generator, &e);
      hazard_tx_process_event(&e);
      led_manager_process_event(&e);
      race_switch_fsm_process_event(&s_race_switch_fsm_storage, &e);
      brake_light_control_process_event(&e);
      regen_braking_process_event(&e);
    }
    wait();
  }
  return STATUS_CODE_OK;
}
