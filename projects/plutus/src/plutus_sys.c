#include "plutus_sys.h"
#include <string.h>
#include "crc32.h"
#include "event_queue.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "plutus_calib.h"
#include "plutus_event.h"
#include "soft_timer.h"

// Board-specific details
typedef struct PlutusSysSpecifics {
  SystemCanDevice can_device;
  SystemCanMessage relay_msg;
} PlutusSysSpecifics;

static StatusCode prv_init_common(PlutusSysStorage *storage, PlutusSysType type) {
  // Init modules that both boards share

  // Standard module inits
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  return STATUS_CODE_OK;
}

PlutusSysType plutus_sys_get_type(void) {
  return PLUTUS_SYS_TYPE_MASTER;
}

StatusCode plutus_sys_init(PlutusSysStorage *storage, PlutusSysType type) {
  if (type >= NUM_PLUTUS_SYS_TYPES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(*storage));
  storage->type = type;

  status_ok_or_return(prv_init_common(storage, type));

  if (type == PLUTUS_SYS_TYPE_MASTER) {
    // Master also handles:
    // LTC AFE/ADC
    const LtcAfeSettings afe_settings = {
      .mosi = PLUTUS_CFG_AFE_SPI_MOSI,
      .miso = PLUTUS_CFG_AFE_SPI_MISO,
      .sclk = PLUTUS_CFG_AFE_SPI_SCLK,
      .cs = PLUTUS_CFG_AFE_SPI_CS,

      .spi_port = PLUTUS_CFG_AFE_SPI_PORT,
      .spi_baudrate = PLUTUS_CFG_AFE_SPI_BAUDRATE,
      .adc_mode = PLUTUS_CFG_AFE_MODE,

      .cell_bitset = PLUTUS_CFG_CELL_BITSET_ARR,
      .aux_bitset = PLUTUS_CFG_AUX_BITSET_ARR,
    };
    status_ok_or_return(ltc_afe_init(&storage->ltc_afe, &afe_settings));

    crc32_init();
    flash_init();

    PlutusCalibBlob calib_blob = { 0 };
    calib_init(&calib_blob, sizeof(calib_blob), false);

    const LtcAdcSettings adc_settings = {
      .mosi = PLUTUS_CFG_CURRENT_SENSE_MOSI,  //
      .miso = PLUTUS_CFG_CURRENT_SENSE_MISO,  //
      .sclk = PLUTUS_CFG_CURRENT_SENSE_SCLK,  //
      .cs = PLUTUS_CFG_CURRENT_SENSE_CS,      //

      .spi_port = PLUTUS_CFG_CURRENT_SENSE_SPI_PORT,          //
      .spi_baudrate = PLUTUS_CFG_CURRENT_SENSE_SPI_BAUDRATE,  //
      .filter_mode = LTC_ADC_FILTER_50HZ_60HZ,                //
    };
    status_ok_or_return(
        current_sense_init(&storage->current_sense, &calib_blob.current_calib, &adc_settings));
  }

  return STATUS_CODE_OK;
}
