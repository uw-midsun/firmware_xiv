#include "ltc2484.h"

StatusCode ltc2484_raw_adc_to_uv(uint8_t *spi_data, int32_t *voltage) {
  Ltc2484Response tmp;

  tmp.u8data[0] = spi_data[3];
  tmp.u8data[1] = spi_data[2];
  tmp.u8data[2] = spi_data[1];
  tmp.u8data[3] = spi_data[0];

  // We have an overrange condition when the SIG and MSB bits are both set
  // (see Table 3 on p.16 of the datasheet)
  if ((spi_data[0] & LTC2484_ERROR_CODE_MASK) == LTC2484_OVERRANGE_CODE) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }
  // We have an underrange condition when the SIG and MSB bits are not set and
  // B27-25 are set
  if ((spi_data[0] & LTC2484_ERROR_CODE_MASK) == LTC2484_UNDERRANGE_CODE) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Sign extend tmp.i32data as two's complement
  // We don't care about the following values, so we can shift them out:
  // - The higher 3 bits are the SIG, DMY and MSB
  // - The lower 4 bits are the configuration word from the previous conversion
  int32_t adc_value = ((tmp.i32data << 3) >> 8);

  // Convert the voltage to uV
  // 2^24 * 4092 * 1000 = 6.86523679e13 (which fits in an int64_t)
  // dividing by 2^24 gives 4092000, which fits in an int32_t
  *voltage = (int32_t)(((int64_t)(adc_value)*LTC2484_V_REF_MILLIVOLTS * 1000) >> 24);

  return STATUS_CODE_OK;
}
