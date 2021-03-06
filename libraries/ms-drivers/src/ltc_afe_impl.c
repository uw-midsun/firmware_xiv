#include "ltc_afe_impl.h"
#include <stddef.h>
#include <string.h>
#include "crc15.h"
#include "delay.h"
#include "log.h"
#include "ltc6811.h"

// - 12-bit, 16-bit and 24-bit values are little endian
// - commands and PEC are big endian

static uint16_t s_read_reg_cmd[NUM_LTC_AFE_REGISTERS] = {
  [LTC_AFE_REGISTER_CONFIG] = LTC6811_RDCFG_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_A] = LTC6811_RDCVA_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_B] = LTC6811_RDCVB_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_C] = LTC6811_RDCVC_RESERVED,
  [LTC_AFE_REGISTER_CELL_VOLTAGE_D] = LTC6811_RDCVD_RESERVED,
  [LTC_AFE_REGISTER_AUX_A] = LTC6811_RDAUXA_RESERVED,
  [LTC_AFE_REGISTER_AUX_B] = LTC6811_RDAUXB_RESERVED,
  [LTC_AFE_REGISTER_STATUS_A] = LTC6811_RDSTATA_RESERVED,
  [LTC_AFE_REGISTER_STATUS_B] = LTC6811_RDSTATB_RESERVED,
  [LTC_AFE_REGISTER_READ_COMM] = LTC6811_RDCOMM_RESERVED,
  [LTC_AFE_REGISTER_START_COMM] = LTC6811_STCOMM_RESERVED
};

static uint8_t s_voltage_reg[NUM_LTC_AFE_VOLTAGE_REGISTERS] = {
  [LTC_AFE_VOLTAGE_REGISTER_A] = LTC_AFE_REGISTER_CELL_VOLTAGE_A,
  [LTC_AFE_VOLTAGE_REGISTER_B] = LTC_AFE_REGISTER_CELL_VOLTAGE_B,
  [LTC_AFE_VOLTAGE_REGISTER_C] = LTC_AFE_REGISTER_CELL_VOLTAGE_C,
  [LTC_AFE_VOLTAGE_REGISTER_D] = LTC_AFE_REGISTER_CELL_VOLTAGE_D,
};

static StatusCode prv_write_config(LtcAfeStorage *afe, uint8_t gpio_enable_pins);

static void prv_wakeup_idle(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;
  // Wakeup method 2 - pair of long -1, +1 for each device
  for (size_t i = 0; i < settings->num_devices; i++) {
    gpio_set_state(&settings->cs, GPIO_STATE_LOW);
    gpio_set_state(&settings->cs, GPIO_STATE_HIGH);
    // Wait for 300us - greater than tWAKE, less than tIDLE
    delay_us(300);
  }
}

static StatusCode prv_build_cmd(uint16_t command, uint8_t *cmd, size_t len) {
  if (len != LTC6811_CMD_SIZE) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  cmd[0] = (uint8_t)(command >> 8);
  cmd[1] = (uint8_t)(command & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);

  return STATUS_CODE_OK;
}

static StatusCode prv_read_register(LtcAfeStorage *afe, LtcAfeRegister reg, uint8_t *data,
                                    size_t len) {
  if (reg > NUM_LTC_AFE_REGISTERS) {
    LOG_DEBUG("prv_read_register bad args\n");
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t reg_cmd = s_read_reg_cmd[reg];

  uint8_t cmd[LTC6811_CMD_SIZE] = { 0 };
  prv_build_cmd(reg_cmd, cmd, LTC6811_CMD_SIZE);

  prv_wakeup_idle(afe);
  return spi_exchange(afe->settings.spi_port, cmd, LTC6811_CMD_SIZE, data, len);
}

// read from a voltage register
static StatusCode prv_read_voltage(LtcAfeStorage *afe, LtcAfeVoltageRegister reg,
                                   LtcAfeVoltageRegisterGroup *data) {
  if (reg > NUM_LTC_AFE_VOLTAGE_REGISTERS) {
    LOG_DEBUG("prv_read_voltage bad args\n");
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memcpy(afe->discharge_bitset, afe->prev_discharges, sizeof(uint16_t) * LTC_AFE_MAX_DEVICES);
  prv_write_config(afe, 0);

  size_t len = sizeof(LtcAfeVoltageRegisterGroup) * afe->settings.num_devices;
  return prv_read_register(afe, s_voltage_reg[reg], (uint8_t *)data, len);
}

// start cell voltage conversion
static StatusCode prv_trigger_adc_conversion(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;
  uint8_t mode = (uint8_t)((settings->adc_mode + 1) % 3);
  // ADCV command
  uint16_t adcv = LTC6811_ADCV_RESERVED | LTC6811_ADCV_DISCHARGE_NOT_PERMITTED |
                  LTC6811_CNVT_CELL_ALL | (mode << 7);

  uint8_t cmd[LTC6811_CMD_SIZE] = { 0 };
  prv_build_cmd(adcv, cmd, LTC6811_CMD_SIZE);

  memcpy(afe->prev_discharges, afe->discharge_bitset, sizeof(uint16_t) * LTC_AFE_MAX_DEVICES);
  memset(afe->discharge_bitset, 0, sizeof(uint16_t) * LTC_AFE_MAX_DEVICES);
  prv_write_config(afe, 0);

  prv_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, LTC6811_CMD_SIZE, NULL, 0);
}

static StatusCode prv_trigger_aux_adc_conversion(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;
  uint8_t mode = (uint8_t)((settings->adc_mode + 1) % 3);
  // ADAX
  uint16_t adax = LTC6811_ADAX_RESERVED | LTC6811_ADAX_GPIO1 | (mode << 7);

  uint8_t cmd[LTC6811_CMD_SIZE] = { 0 };
  prv_build_cmd(adax, cmd, LTC6811_CMD_SIZE);

  prv_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, cmd, LTC6811_CMD_SIZE, NULL, 0);
}

static StatusCode prv_aux_write_comm_register(LtcAfeStorage *afe, uint8_t therm) {
  if (therm >= AUX_ADG731_NUM_PINS) {
    LOG_DEBUG("aux out of range\n");
    return STATUS_CODE_OUT_OF_RANGE;
  }
  LtcAfeSettings *settings = &afe->settings;
  LtcAfeWriteCommRegPacket packet = { 0 };
  // Build WRCOMM Command
  prv_build_cmd(LTC6811_WRCOMM_RESERVED, packet.wrcomm, LTC6811_CMD_SIZE);
  
  // Same thing as writing the config to all devices
  for (uint8_t dev = 0; dev < settings->num_devices; dev++) {
    packet.devices[dev].reg.icom0 = LTC6811_ICOM_CSBM_LOW;
    packet.devices[dev].reg.d0_msb = LTC6811_EXTRACT_DATA_MSB(therm);
    packet.devices[dev].reg.d0_lsb = LTC6811_EXTRACT_DATA_LSB(therm);
    packet.devices[dev].reg.fcom0 = LTC6811_FCOM_CSBM_HIGH;
    packet.devices[dev].reg.icom1 = LTC6811_ICOM_NO_TRANSMIT;
    packet.devices[dev].reg.icom2 = LTC6811_ICOM_NO_TRANSMIT;
    uint16_t comm_pec = crc15_calculate((uint8_t *)&packet.devices[dev].reg,
                                        sizeof(LtcAfeCommRegisterData));
    packet.devices[dev].pec = SWAP_UINT16(comm_pec);
  }

  size_t len = SIZEOF_LTC_AFE_WRITE_COMM_PACKET(settings->num_devices);

  prv_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, (uint8_t *)&packet, len, NULL, 0);
}

static StatusCode prv_aux_send_comm_register(LtcAfeStorage *afe) {
  LtcAfeSettings *settings = &afe->settings;
  LtcAfeSendCommRegPacket packet = { 0 };
  // Build STCOMM command
  prv_build_cmd(LTC6811_STCOMM_RESERVED, packet.stcomm, LTC6811_CMD_SIZE);
  for (uint8_t i = 0; i < LTC6811_NUM_COMM_REG_BYTES; i++) {
    // NULL bytes so our SPI drivers will send 24 clock cycles
    packet.clk[i] = 0;
  }
  prv_wakeup_idle(afe);
  return spi_exchange(settings->spi_port, (uint8_t *)&packet, sizeof(LtcAfeSendCommRegPacket), NULL,
                      0);
}

// write config to all devices
static StatusCode prv_write_config(LtcAfeStorage *afe, uint8_t gpio_enable_pins) {
  LtcAfeSettings *settings = &afe->settings;
  // see p.54 in datasheet
  LtcAfeWriteConfigPacket config_packet = { 0 };

  prv_build_cmd(LTC6811_WRCFG_RESERVED, config_packet.wrcfg, SIZEOF_ARRAY(config_packet.wrcfg));

  // essentially, each set of CFGR registers are clocked through each device,
  // until the first set reaches the last device (like a giant shift register)
  // thus, we send CFGR registers starting with the bottom slave in the stack
  for (uint8_t curr_device = 0; curr_device < settings->num_devices; curr_device++) {
    uint8_t enable = gpio_enable_pins;

    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;

    config_packet.devices[curr_device].reg.discharge_bitset = afe->discharge_bitset[curr_device];
    config_packet.devices[curr_device].reg.discharge_timeout = LTC_AFE_DISCHARGE_TIMEOUT_30_S;

    config_packet.devices[curr_device].reg.adcopt = ((settings->adc_mode + 1) > 3);
    config_packet.devices[curr_device].reg.dten = true;

    config_packet.devices[curr_device].reg.undervoltage = undervoltage;
    config_packet.devices[curr_device].reg.overvoltage = overvoltage;

    // GPIO 1 is used to read data from the mux
    config_packet.devices[curr_device].reg.gpio = (enable >> 3);

    uint16_t cfgr_pec = crc15_calculate((uint8_t *)&config_packet.devices[curr_device].reg, 6);
    config_packet.devices[curr_device].pec = SWAP_UINT16(cfgr_pec);
  }

  size_t len = SIZEOF_LTC_AFE_WRITE_CONFIG_PACKET(settings->num_devices);
  // for (uint8_t i = 0; i < len; i++) {
  //   LOG_DEBUG("byte %d: 0x%x\n", i, ((uint8_t *)&config_packet)[i]);
  // }
  prv_wakeup_idle(afe);
  uint8_t status = spi_exchange(settings->spi_port, (uint8_t *)&config_packet, len, NULL, 0);
  // exit(0);
  return status;
}

static void prv_calc_offsets(LtcAfeStorage *afe) {
  // Our goal is to populate result arrays as if the ignored inputs don't exist. This requires
  // converting the actual LTC6811 cell index to some potentially smaller result index.
  //
  // Since we access the same register across multiple devices, we can't just keep a counter and
  // increment it for each new value we get during register access. Instead, we precompute each
  // input's corresponding result index. Inputs that are ignored will not be copied into the result
  // array.
  //
  // Similarly, we do the opposite mapping for discharge.
  LtcAfeSettings *settings = &afe->settings;
  size_t cell_index = 0;
  size_t aux_index = 0;
  for (size_t device = 0; device < settings->num_devices; device++) {
    for (size_t device_cell = 0; device_cell < LTC_AFE_MAX_CELLS_PER_DEVICE; device_cell++) {
      size_t cell = device * LTC_AFE_MAX_CELLS_PER_DEVICE + device_cell;

      if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
        // Cell input enabled - store the index that this input should be stored in
        // when copying to the result array and the opposite for discharge
        afe->discharge_cell_lookup[cell_index] = cell;
        afe->cell_result_lookup[cell] = cell_index++;
      }
    }
    for (size_t device_therm = 0; device_therm < LTC_AFE_MAX_THERMISTORS_PER_DEVICE; device_therm++) {
      size_t therm = device * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + device_therm;

      if ((settings->aux_bitset[device] >> device_therm) & 0x1) {
        afe->aux_result_lookup[therm] = aux_index++;
      }
    }
  }
}

StatusCode ltc_afe_impl_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  if (settings->num_devices > LTC_AFE_MAX_DEVICES ||
      settings->num_cells > settings->num_devices * LTC_AFE_MAX_CELLS_PER_DEVICE ||
      settings->num_thermistors > settings->num_devices * LTC_AFE_MAX_THERMISTORS_PER_DEVICE) {
    // bad no. devices (needs code change)
    // bad no. of cells (needs verification)
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(afe, 0, sizeof(*afe));
  memcpy(&afe->settings, settings, sizeof(afe->settings));

  prv_calc_offsets(afe);
  crc15_init_table();

  SpiSettings spi_config = {
    .baudrate = settings->spi_baudrate,  //
    .mode = SPI_MODE_3,                  //
    .mosi = settings->mosi,              //
    .miso = settings->miso,              //
    .sclk = settings->sclk,              //
    .cs = settings->cs,
  };
  spi_init(settings->spi_port, &spi_config);

  // Use GPIO1 as analog input, GPIO 3-5 for SPI
  uint8_t gpio_bits = LTC6811_GPIO1_PD_OFF | LTC6811_GPIO2_PD_ON | LTC6811_GPIO3_PD_ON |
                      LTC6811_GPIO4_PD_ON | LTC6811_GPIO5_PD_ON;
  return prv_write_config(afe, gpio_bits);
}

StatusCode ltc_afe_impl_trigger_cell_conv(LtcAfeStorage *afe) {
  return prv_trigger_adc_conversion(afe);
}

StatusCode ltc_afe_impl_trigger_aux_conv(LtcAfeStorage *afe, uint8_t thermistor) {
  uint8_t gpio_bits =
      LTC6811_GPIO1_PD_OFF | LTC6811_GPIO3_PD_OFF | LTC6811_GPIO4_PD_OFF | LTC6811_GPIO5_PD_OFF;
  prv_write_config(afe, gpio_bits);
  prv_aux_write_comm_register(afe, thermistor);
  prv_aux_send_comm_register(afe);
  return prv_trigger_aux_adc_conversion(afe);
}

StatusCode ltc_afe_impl_read_cells(LtcAfeStorage *afe) {
  // Read all voltage A, then B, ...
  LtcAfeSettings *settings = &afe->settings;
  for (uint8_t cell_reg = 0; cell_reg < NUM_LTC_AFE_VOLTAGE_REGISTERS; ++cell_reg) {
    LtcAfeVoltageRegisterGroup voltage_register[LTC_AFE_MAX_DEVICES] = { 0 };
    prv_read_voltage(afe, cell_reg, voltage_register);
    // uint8_t *d = &voltage_register[0].reg.values[0];
    // LOG_DEBUG("data: 0x%x, 0x%x, 0x%x, 0x%x\n", *d, *(d+1), *(d+2), *(d+3));
    for (uint8_t device = 0; device < settings->num_devices; ++device) {
      for (uint16_t cell = 0; cell < LTC6811_CELLS_IN_REG; ++cell) {
        // LSB of the reading is 100 uV
        uint16_t voltage = voltage_register[device].reg.voltages[cell];
        uint16_t device_cell = cell + (cell_reg * LTC6811_CELLS_IN_REG);
        uint16_t index = device * LTC_AFE_MAX_CELLS_PER_DEVICE + device_cell;

        if ((settings->cell_bitset[device] >> device_cell) & 0x1) {
          // Input enabled - store result
          afe->cell_voltages[afe->cell_result_lookup[index]] = voltage;
        }
      }

      // the Packet Error Code is transmitted after the cell data (see p.45)
      uint16_t received_pec = SWAP_UINT16(voltage_register[device].pec);
      uint16_t data_pec = crc15_calculate((uint8_t *)&voltage_register[device], 6);
      if (received_pec != data_pec) {
        // return early on failure
        LOG_DEBUG("PEC error, got 0x%x but expected 0x%x\n", received_pec, data_pec);
        uint8_t *d = &voltage_register[0].reg.values[0];
        LOG_DEBUG("data: 0x%x, 0x%x, 0x%x, 0x%x\n", *d, *(d + 1), *(d + 2), *(d + 3));
        return status_code(STATUS_CODE_INTERNAL_ERROR);
      }
    }
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_impl_read_aux(LtcAfeStorage *afe, uint8_t thermistor) {
  LtcAfeSettings *settings = &afe->settings;
  LtcAfeAuxRegisterGroupPacket register_data[LTC_AFE_MAX_DEVICES] = { 0 };

  size_t len = settings->num_devices * sizeof(LtcAfeAuxRegisterGroupPacket);
  prv_read_register(afe, LTC_AFE_REGISTER_AUX_A, (uint8_t *)register_data, len);

  for (uint16_t device = 0; device < settings->num_devices; ++device) {
    // data comes in in the form { 1, 1, 2, 2, 3, 3, PEC, PEC }
    // we only care about GPIO1 and the PEC
    uint16_t voltage = register_data[device].reg.voltages[0];

    if ((settings->aux_bitset[device] >> thermistor) & 0x1) {
      // Input enabled - store result
      uint16_t index = device * LTC_AFE_MAX_THERMISTORS_PER_DEVICE + thermistor;
      afe->aux_voltages[afe->aux_result_lookup[index]] = voltage;
    }

    uint16_t received_pec = SWAP_UINT16(register_data[device].pec);
    uint16_t data_pec = crc15_calculate((uint8_t *)&register_data[device], 6);
    if (received_pec != data_pec) {
      LOG_DEBUG("aux PEC error\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_impl_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  if (cell >= afe->settings.num_cells) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t actual_cell = afe->discharge_cell_lookup[cell];
  uint16_t device_cell = actual_cell % LTC_AFE_MAX_CELLS_PER_DEVICE;
  uint16_t device = actual_cell / LTC_AFE_MAX_CELLS_PER_DEVICE;

  afe->discharge_bitset[device] = 0;

  if (discharge) {
    afe->discharge_bitset[device] |= (1 << device_cell);
  } else {
    afe->discharge_bitset[device] &= ~(1 << device_cell);
  }

  return STATUS_CODE_OK;
}
