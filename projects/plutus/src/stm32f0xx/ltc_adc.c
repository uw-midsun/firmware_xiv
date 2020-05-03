#include "ltc_adc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "critical_section.h"
#include "delay.h"
#include "ltc2484.h"
#include "soft_timer.h"

uint8_t s_filter_modes[NUM_LTC_ADC_FILTER_MODES] = {
  LTC2484_REJECTION_50HZ_60HZ,
  LTC2484_REJECTION_50HZ,
  LTC2484_REJECTION_60HZ,
};

static void prv_toggle_pin_altfn(GpioAddress addr, bool enable) {
  GpioSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_0,
  };

  if (enable) {
    settings.alt_function = GPIO_ALTFN_0;
  } else {
    settings.alt_function = GPIO_ALTFN_NONE;
  }

  gpio_init_pin(&addr, &settings);
}

static void prv_ltc_adc_read(SoftTimerId timer_id, void *context) {
  LtcAdcStorage *storage = (LtcAdcStorage *)context;

  // Pull CS low so we can check for MISO to go low, signalling that the
  // conversion is now complete
  gpio_set_state(&storage->cs, GPIO_STATE_LOW);

  // Disable the Alt Fn on MISO so we can read the value
  prv_toggle_pin_altfn(storage->miso, false);

  // According to the Timing Characteristics (p.5 in the datasheet), we should
  // expect 149.9ms for conversion time (in the worst case).
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&storage->miso, &state);

  // Restore CS high so we can trigger a SPI exchange
  gpio_set_state(&storage->cs, GPIO_STATE_HIGH);

  // Restore the Alt Fn of the MISO pin
  prv_toggle_pin_altfn(storage->miso, true);

  if (state != GPIO_STATE_LOW) {
    // MISO should have gone low, signaling that the conversion has finished
    storage->buffer.status = status_code(STATUS_CODE_TIMEOUT);

    // Pass null pointer into callback to indicate invalid data
    if (storage->fault_callback != NULL) {
      storage->fault_callback(storage->fault_context);
    }
  } else {
    // Keep the previous mode and don't do anything special (ie. send a command
    // byte equal to 0). Since our SPI driver sends 0x00 by default, we can
    // just send NULL.
    //
    // Due to the way our SPI driver works, we send NULL bytes in order to
    // ensure that 4 bytes are exchanged in total.
    uint8_t result[4] = { 0 };
    StatusCode status = spi_exchange(storage->spi_port, NULL, 0, result, 4);

    storage->buffer.status = ltc2484_raw_adc_to_uv(result, &storage->buffer.value);

    // Invoke callback with the new data
    if (storage->callback != NULL) {
      storage->callback(&storage->buffer.value, storage->context);
    }
  }

  soft_timer_start_millis(LTC2484_MAX_CONVERSION_TIME_MS, prv_ltc_adc_read, storage,
                          &storage->buffer.timer_id);
}

StatusCode ltc_adc_init(LtcAdcStorage *storage, const LtcAdcSettings *settings) {
  if (storage == NULL || settings->filter_mode >= NUM_LTC_ADC_FILTER_MODES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->buffer.status = STATUS_CODE_UNINITIALIZED;
  storage->buffer.value = INT16_MAX;
  storage->callback = NULL;
  storage->fault_callback = NULL;
  storage->context = NULL;
  storage->fault_context = NULL;
  storage->spi_port = settings->spi_port;
  storage->cs = settings->cs;
  storage->miso = settings->miso;

  // The LTC2484 uses SPI Mode 0 (see Figure 5 on p.20 in the datasheet)
  SpiSettings spi_config = {
    .baudrate = settings->spi_baudrate,
    .mode = SPI_MODE_0,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
  };

  spi_init(storage->spi_port, &spi_config);

  uint8_t input[1] = { LTC2484_ENABLE | LTC2484_EXTERNAL_INPUT | LTC2484_AUTO_CALIBRATION |
                       s_filter_modes[settings->filter_mode] };
  // send config
  spi_exchange(storage->spi_port, input, 1, NULL, 0);

  // Wait for at least 200ms before attempting another read
  //
  // This hacky solution is to get around the fact that isoSPI seems to require
  // a clock signal in order to encode a change in state on the MISO pin. Since
  // the LTC2484 uses the MISO pin to signal that the EOC has completed
  // (without requiring a clock signal), we are unable to test that correctly,
  // and thus we are forced to delay for the maximum conversion time before
  // performing another read.
  return soft_timer_start_millis(LTC2484_MAX_CONVERSION_TIME_MS, prv_ltc_adc_read, storage,
                                 &storage->buffer.timer_id);
}

StatusCode ltc_adc_register_callback(LtcAdcStorage *storage, LtcAdcCallback callback,
                                     void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->callback = callback;
  storage->context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode ltc_adc_register_fault_callback(LtcAdcStorage *storage,
                                           LtcAdcFaultCallback fault_callback, void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->fault_callback = fault_callback;
  storage->fault_context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

void test_ltc_adc_set_input_voltage(int32_t input_voltage) {}

void test_ltc_adc_set_fault_status(bool fault_state) {}
