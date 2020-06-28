#include "ads1259_adc.h"
#include "ads1259_adc_defs.h"
#include "delay.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "math.h"

#define NUM_ADS_RX_BYTES 4
#define NUM_CONFIG_REGISTERS 3
#define NUM_REGISTER_WRITE_COMM 5
#define CHK_SUM_FLAG_BIT 0x80

// Timer used to collect data after convert command sent
static SoftTimerId s_timer_id;

// Used to determine length of time needed between convert command sent and data collection
static const uint32_t s_conversion_time_ms_lookup[NUM_ADS1259_DATA_RATE] = {
  [ADS1259_DATA_RATE_10] = 100, [ADS1259_DATA_RATE_17] = 61,   [ADS1259_DATA_RATE_50] = 21,
  [ADS1259_DATA_RATE_60] = 17,  [ADS1259_DATA_RATE_400] = 3,   [ADS1259_DATA_RATE_1200] = 2,
  [ADS1259_DATA_RATE_3600] = 1, [ADS1259_DATA_RATE_14400] = 1,
};

static const uint32_t s_calibration_time_ms_lookup[NUM_ADS1259_DATA_RATE] = {
  [ADS1259_DATA_RATE_10] = 1900, [ADS1259_DATA_RATE_17] = 1140, [ADS1259_DATA_RATE_50] = 380,
  [ADS1259_DATA_RATE_60] = 318,  [ADS1259_DATA_RATE_400] = 49,  [ADS1259_DATA_RATE_1200] = 17,
  [ADS1259_DATA_RATE_3600] = 7,  [ADS1259_DATA_RATE_14400] = 3,
};

// Number of noise free bits for each sampling rate
static const uint8_t s_num_usable_bits[NUM_ADS1259_DATA_RATE] = {
  [ADS1259_DATA_RATE_10] = 21, [ADS1259_DATA_RATE_17] = 21, [ADS1259_DATA_RATE_50] = 20,
  [ADS1259_DATA_RATE_60] = 20,  [ADS1259_DATA_RATE_400] = 19,  [ADS1259_DATA_RATE_1200] = 18,
  [ADS1259_DATA_RATE_3600] = 17,  [ADS1259_DATA_RATE_14400] = 16,
};

// tx spi command to ads1259
static void prv_send_command(Ads1259Storage *storage, uint8_t command) {
  uint8_t payload[] = { command };
  spi_exchange(storage->spi_port, payload, 1, NULL, 0);
}

// Reads 1-byte reg value to storage->data
static StatusCode prv_check_register(Ads1259Storage *storage, uint8_t reg_add, uint8_t reg_val) {
  uint8_t payload[] = { (ADS1259_READ_REGISTER | reg_add) };
  spi_exchange(storage->spi_port, payload, 1, &storage->rx_data.MSB, 1 );
  if (reg_val != storage->rx_data.MSB) return STATUS_CODE_UNINITIALIZED;
  return STATUS_CODE_OK;
}

static StatusCode prv_configure_registers(Ads1259Storage *storage) {
  uint8_t register_lookup[NUM_CONFIG_REGISTERS] = {
    (ADS1259_INTERNAL_REF_BIAS_ENABLE | ADS1259_SPI_TIMEOUT_ENABLE),
    (ADS1259_OUT_OF_RANGE_FLAG_ENABLE | ADS1259_CHECK_SUM_ENABLE),
    (ADS1259_CONVERSION_CONTROL_MODE_PULSE | ADS1259_DATA_RATE_SPS),
  };
  // reset all register values to default
  prv_send_command(storage, ADS1259_RESET);  // Needs 8 fclk cycles before next command
  uint8_t payload[NUM_REGISTER_WRITE_COMM] = { (ADS1259_WRITE_REGISTER | ADS1259_ADDRESS_CONFIG0),
                                               NUM_CONFIG_REGISTERS, register_lookup[0],
                                               register_lookup[1], register_lookup[2] };
  // tx write-reg command and data for all three config registers
  spi_exchange(storage->spi_port, payload, NUM_REGISTER_WRITE_COMM, NULL, 0);
  // sanity check that data was written correctly
  for (int reg = 0; reg < NUM_CONFIG_REGISTERS; reg++) {
    status_ok_or_return(prv_check_register(storage, reg, register_lookup[reg]));
  }
  return STATUS_CODE_OK;
}

//using the amount of noise free bits based on the SPS and VREF calculate analog voltage value
static void prv_convert_data(Ads1259Storage* storage) {
    uint32_t resolution = pow(2, s_num_usable_bits[ADS1259_DATA_RATE_SPS]);
    storage->reading = (storage->conv_data.raw >> (24 - s_num_usable_bits[ADS1259_DATA_RATE_SPS]))*ADS1259_VREF_EXTERNAL / resolution;
}
// calculate check-sum based on page 29 of datasheet
static StatusCode prv_checksum(Ads1259Storage *storage) {
  uint8_t sum = (uint8_t)(storage->rx_data.LSB + storage->rx_data.MID +
                          storage->rx_data.MSB + ADS1259_CHECKSUM_OFFSET);
  if (storage->rx_data.CHK_SUM & CHK_SUM_FLAG_BIT) return STATUS_CODE_OUT_OF_RANGE;
  if ((sum &= ~(CHK_SUM_FLAG_BIT)) != (storage->rx_data.CHK_SUM &= ~(CHK_SUM_FLAG_BIT))) {
    return STATUS_CODE_INTERNAL_ERROR;
  }
  return STATUS_CODE_OK;
}

static void prv_conversion_callback(SoftTimerId timer_id, void *context) {
  Ads1259Storage *storage = (Ads1259Storage *)context;
  uint8_t payload[] = { ADS1259_READ_DATA_BY_OPCODE };
    spi_exchange(storage->spi_port, payload, 1, (uint8_t*)&storage->rx_data, NUM_ADS_RX_BYTES);
  if(prv_checksum(storage)) {
      //error handler
  }
  storage->conv_data.MSB = storage->rx_data.LSB;
  storage->conv_data.MID = storage->rx_data.MID;
  storage->conv_data.LSB = storage->rx_data.MSB;
  prv_convert_data(storage);
}

// Initializes ads1259 connection on a SPI port. Can be re-called to calibrate adc
StatusCode ads1259_init(Ads1259Settings *settings, Ads1259Storage *storage) {
  storage->spi_port = settings->spi_port;
  const SpiSettings spi_settings = {
    .baudrate = settings->spi_baudrate,
    .mode = SPI_MODE_1,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
  };
  status_ok_or_return(spi_init(settings->spi_port, &spi_settings));
  delay_us(100);
  // first command that must be sent on power-up before registers can be read
  prv_send_command(storage, ADS1259_STOP_READ_DATA_CONTINUOUS);
  status_ok_or_return(prv_configure_registers(storage));
  prv_send_command(storage, ADS1259_OFFSET_CALIBRATION);
  soft_timer_start_millis(s_calibration_time_ms_lookup[ADS1259_DATA_RATE_SPS], NULL, NULL,
                          &s_timer_id);
  prv_send_command(storage, ADS1259_GAIN_CALIBRATION);
  soft_timer_start_millis(s_calibration_time_ms_lookup[ADS1259_DATA_RATE_SPS], NULL, NULL,
                          &s_timer_id);
  return STATUS_CODE_OK;
}

// Reads conversion data to data struct in storage. data->reading gives total value
StatusCode ads1259_get_conversion_data(Ads1259Storage *storage) {
  prv_send_command(storage, ADS1259_START_CONV);
  status_ok_or_return(soft_timer_start_millis(s_conversion_time_ms_lookup[ADS1259_DATA_RATE_SPS],
                                              prv_conversion_callback, storage, &s_timer_id));
  status_ok_or_return(prv_checksum(storage));
  prv_convert_data(storage);
  return STATUS_CODE_OK;
}
