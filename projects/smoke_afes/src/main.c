#include "afe_setup.h"
#include "delay.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "soft_timer.h"
#include "status.h"

// Test setup (do change)
#define NUM_AFES 3
#define CELLS_PER_AFE 12
#define THERMS_PER_AFE 0
#define CELL_BITSET 0xFFF        // 12 bits for 12 cells
#define THERM_BITSET 0x0  // 32 bits for 32 therms, also ~(uint32_t)0

// Optional delay between readings
#define READ_DELAY_MS 2000

// Features to enable
#define LOG_VOLTS true
#define LOG_TEMPS false
#define PASSIVE_BALANCE true
#define PASSIVE_BALANCE_DIFF_MV 25

static LtcAfeStorage s_afe = { 0 };

static void prv_log_table(uint16_t *results, uint16_t len, const char *name) {
  for (uint16_t row = 0; row < len / NUM_AFES; row++) {
    printf("AFE %d  ", row / (len / NUM_AFES / NUM_AFES));
    for (uint16_t col = 0; col < NUM_AFES; col++) {
      uint16_t index = row * NUM_AFES + col;
      printf("%s#%02d = %05d  ", name, index % (len / NUM_AFES), results[index]);
    }
    printf("\n");
  }
}

static void prv_log_volts(uint16_t *results, uint16_t len, void *context) {
  ltc_afe_request_aux_conversion(&s_afe);

  bool to_balance[CELLS_PER_AFE * NUM_AFES] = { 0 };

  for (uint8_t dev = 0; dev < NUM_AFES; dev++) {
    uint16_t cell_max = dev * CELLS_PER_AFE;
    uint16_t cell_min = dev * CELLS_PER_AFE;
    for (uint16_t cell = 0; cell < CELLS_PER_AFE; cell++) {
      uint16_t idx = cell + dev * CELLS_PER_AFE;
      if (results[cell_max] < results[idx]) {
        cell_max = idx;
      }
      if (results[cell_min] > results[idx]) {
        cell_min = idx;
      }
    }
    if (results[cell_max] - results[cell_min] >= PASSIVE_BALANCE_DIFF_MV * 10) {
      to_balance[cell_max] = true;
    }
    LOG_DEBUG("AFE %d max(%d)-min(%d) diff: %d\n", dev, cell_max - (dev*CELLS_PER_AFE),
      cell_min - (dev*CELLS_PER_AFE), results[cell_max] - results[cell_min]);
  }

  if (PASSIVE_BALANCE) {
    for (uint16_t i = 0; i < len; i++) {
      ltc_afe_toggle_cell_discharge(&s_afe, i, to_balance[i]);
    }
  }

  if (!LOG_VOLTS) {
    return;
  }

  LOG_DEBUG("Voltage logging\n");
  prv_log_table(results, len, "cell");
  for (uint8_t i = 0; i < NUM_AFES; i++) {
    int16_t cell_num = __builtin_ctz(s_afe.discharge_bitset[i]);
    printf("AFE %d discharge bitset: 0x%03x = cell #%d\n", i, s_afe.discharge_bitset[i], cell_num);
  }

  delay_ms(READ_DELAY_MS);
}

static void prv_log_temps(uint16_t *results, uint16_t len, void *context) {
  ltc_afe_request_cell_conversion(&s_afe);

  if (!LOG_TEMPS) {
    return;
  }

  LOG_DEBUG("Temp logging\n");
  prv_log_table(results, len, "therm");

  delay_ms(READ_DELAY_MS);
}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  // afe initialization
  LtcAfeSettings settings = {
    .mosi = AFE_SPI_MOSI,
    .miso = AFE_SPI_MISO,
    .sclk = AFE_SPI_SCLK,
    .cs = AFE_SPI_CS,
    .spi_port = AFE_SPI_PORT,
    .spi_baudrate = AFE_SPI_BAUDRATE,

    .adc_mode = AFE_MODE,
    .cell_bitset = { 0 },
    .aux_bitset = { 0 },
    .num_devices = NUM_AFES,
    .num_cells = CELLS_PER_AFE * NUM_AFES,
    .num_thermistors = THERMS_PER_AFE * NUM_AFES,
    .ltc_events = { .trigger_cell_conv_event = AFE_TRIGGER_CELL_CONV_EVENT,
                    .cell_conv_complete_event = AFE_CELL_CONV_COMPLETE_EVENT,
                    .trigger_aux_conv_event = AFE_TRIGGER_AUX_CONV_EVENT,
                    .aux_conv_complete_event = AFE_AUX_CONV_COMPLETE_EVENT,
                    .callback_run_event = AFE_CALLBACK_RUN_EVENT,
                    .fault_event = AFE_FAULT_EVENT },
    .cell_result_cb = prv_log_volts,
    .aux_result_cb = prv_log_temps,
    .result_context = NULL,
  };
  for (uint8_t i = 0; i < NUM_AFES; i++) {
    settings.cell_bitset[i] = CELL_BITSET;
    settings.aux_bitset[i] = THERM_BITSET;
  }
  StatusCode init_status = ltc_afe_init(&s_afe, &settings);
  if (init_status != STATUS_CODE_OK) {
    LOG_DEBUG("Error durring init :( exiting with code %d\n", init_status);
    exit(0);
  }
  LOG_DEBUG("AFEs inited!\n");

  // Starting!
  ltc_afe_request_cell_conversion(&s_afe);

  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      ltc_afe_process_event(&s_afe, &e);
    }
  }
}
