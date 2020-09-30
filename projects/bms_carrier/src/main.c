#include "bms.h"
#include "bms_events.h"
#include "can.h"
#include "can_handler.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "fault_bps.h"
#include "interrupt.h"
#include "killswitch.h"
#include "soft_timer.h"

static CanStorage s_can_storage = { 0 };
static BmsStorage s_bms_storage = { 0 };

void prv_setup_system_can() {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = BMS_CAN_EVENT_RX,
    .tx_event = BMS_CAN_EVENT_TX,
    .fault_event = BMS_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  gpio_init();
  gpio_it_init();
  prv_setup_system_can();

  I2CSettings i2c_settings = {
    .scl = BMS_PERIPH_I2C_SCL_PIN,
    .sda = BMS_PERIPH_I2C_SDA_PIN,
    .speed = I2C_SPEED_FAST,
  };
  i2c_init(BMS_PERIPH_I2C_PORT, &i2c_settings);

  // initialize modules
  bps_heartbeat_init(&s_bms_storage.bps_storage, BPS_HB_FREQ_MS);
  can_handler_init(&s_bms_storage, TIME_BETWEEN_TX_IN_MILLIS);
  // TODO(SOFT-61): fill in these values from hardware
  CellSenseSettings cell_settings = {
    .undervoltage_dmv = 0,
    .overvoltage_dmv = 0,
    .charge_overtemp_dmv = 0,
    .discharge_overtemp_dmv = 0,
  };
  cell_sense_init(&cell_settings, &s_bms_storage.afe_readings, &s_bms_storage.ltc_afe_storage);
  SpiSettings spi_settings = {
    .baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
  };
  current_sense_init(&s_bms_storage.current_storage, &spi_settings, CONVERSION_TIME_MS);
  FanControlSettings fan_settings = {
    .callback = NULL,
    .callback_context = NULL,
    .i2c_settings = i2c_settings,
    .i2c_write_addr = BMS_FAN_CTRL_1_I2C_ADDR,
    .i2c_read_addr = BMS_FAN_CTRL_1_I2C_ADDR,
  };
  // We have two fan controllers
  fan_control_init(&fan_settings, &s_bms_storage.fan_storage_1);
  fan_settings.i2c_write_addr = BMS_FAN_CTRL_2_I2C_ADDR;
  fan_settings.i2c_read_addr = BMS_FAN_CTRL_2_I2C_ADDR;
  fan_control_init(&fan_settings, &s_bms_storage.fan_storage_2);
  fault_bps_init(&s_bms_storage);
  killswitch_init(&s_bms_storage.killswitch_storage);
  relay_sequence_init(&s_bms_storage.relay_storage);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      cell_sense_process_event(&e);
    }
  }

  return 0;
}
