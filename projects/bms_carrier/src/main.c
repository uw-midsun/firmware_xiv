#include "bms.h"
#include "bms_events.h"
#include "can.h"
#include "can_handler.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "fault_bps.h"
#include "interrupt.h"
#include "killswitch.h"
#include "mcp23008_gpio_expander.h"
#include "soft_timer.h"
#include "wait.h"

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
  // bps_heartbeat_init(&s_bms_storage.bps_storage, BPS_HB_FREQ_MS);
  // can_handler_init(&s_bms_storage, TIME_BETWEEN_TX_IN_MILLIS);
  LtcAfeSettings afe_settings = {
    // Settings pending hardware validation
    .cs = AFE_SPI_SS,
    .mosi = AFE_SPI_MOSI,
    .miso = AFE_SPI_MISO,
    .sclk = AFE_SPI_SCK,

    .spi_port = AFE_SPI_PORT,
    .spi_baudrate = 750000,

    .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

    .cell_bitset = { 0xFFF, 0xFFF },
    .aux_bitset = { 0xFFFFFFFF, 0xFFFFFFFF },

    .num_devices = NUM_AFES,
    .num_cells = NUM_TOTAL_CELLS,
    .num_thermistors = NUM_THERMISTORS,

    .ltc_events =
        {
            .trigger_cell_conv_event = BMS_AFE_EVENT_TRIGGER_CELL_CONV,    //
            .cell_conv_complete_event = BMS_AFE_EVENT_CELL_CONV_COMPLETE,  //
            .trigger_aux_conv_event = BMS_AFE_EVENT_TRIGGER_AUX_CONV,      //
            .aux_conv_complete_event = BMS_AFE_EVENT_AUX_CONV_COMPLETE,    //
            .callback_run_event = BMS_AFE_EVENT_CALLBACK_RUN,              //
            .fault_event = BMS_AFE_EVENT_FAULT                             //
        },

    .cell_result_cb = NULL,  // These callbacks are set later on by cell sense
    .aux_result_cb = NULL,
  };
  ltc_afe_init(&s_bms_storage.ltc_afe_storage, &afe_settings);
  // TODO(SOFT-61): fill in these values from hardware
  CellSenseSettings cell_settings = {
    .undervoltage_dmv = 25000,
    .overvoltage_dmv = 42000,
    .charge_overtemp_dmv = 0xFFFF,  // These values will be tested in hardware eventually
    .discharge_overtemp_dmv = 0xFFFF,
  };
  cell_sense_init(&cell_settings, &s_bms_storage.afe_readings, &s_bms_storage.ltc_afe_storage);
  // SpiSettings spi_settings = {
  //   .baudrate = 600000,
  //   .mosi = { .port = GPIO_PORT_B, 15 },
  //   .miso = { .port = GPIO_PORT_B, 14 },
  //   .sclk = { .port = GPIO_PORT_B, 13 },
  //   .cs = { .port = GPIO_PORT_B, 12 },
  // };
  // current_sense_init(&s_bms_storage.current_storage, &spi_settings, CONVERSION_TIME_MS);
  FanControlSettings fan_settings = {
    .callback = NULL,
    .callback_context = NULL,
    .i2c_settings = i2c_settings,
    .i2c_write_addr = BMS_FAN_CTRL_1_I2C_ADDR,
    .i2c_read_addr = BMS_FAN_CTRL_1_I2C_ADDR,
  };
  // We have two fan controllers
  // fan_control_init(&fan_settings, &s_bms_storage.fan_storage_1);
  // fan_settings.i2c_write_addr = BMS_FAN_CTRL_2_I2C_ADDR;
  // fan_settings.i2c_read_addr = BMS_FAN_CTRL_2_I2C_ADDR;
  // fan_control_init(&fan_settings, &s_bms_storage.fan_storage_2);
  fault_bps_init(&s_bms_storage);
  // killswitch_init();
  // mcp23008_gpio_init(BMS_PERIPH_I2C_PORT, BMS_IO_EXPANDER_I2C_ADDR);
  // relay_sequence_init(&s_bms_storage.relay_storage);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      ltc_afe_process_event(&s_bms_storage.ltc_afe_storage, &e);
      cell_sense_process_event(&e);
    }
    wait();
  }

  return 0;
}
