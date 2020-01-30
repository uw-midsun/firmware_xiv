#include "mcp2515.h"
#include <stddef.h>
#include <string.h>
#include "critical_section.h"
#include "gpio_it.h"
#include "log.h"
#include "mcp2515_defs.h"
#include "delay.h"
#include "debug_led.h"
#include "soft_timer.h"
#include "log.h"

#include "delay.h"

#include "critical_section.h"

#define MCP2515_MAX_WRITE_BUFFER_LEN 10

typedef struct Mcp2515TxBuffer {
  uint8_t id;
  uint8_t data;
  uint8_t rts;
} Mcp2515TxBuffer;

typedef struct Mcp2515RxBuffer {
  uint8_t id;
  uint8_t data;
  uint8_t int_flag;
} Mcp2515RxBuffer;

static const Mcp2515TxBuffer s_tx_buffers[] = {
  { .id = MCP2515_LOAD_TXB0SIDH, .data = MCP2515_LOAD_TXB0D0, .rts = MCP2515_RTS_TXB0 },
  { .id = MCP2515_LOAD_TXB1SIDH, .data = MCP2515_LOAD_TXB1D0, .rts = MCP2515_RTS_TXB1 },
  { .id = MCP2515_LOAD_TXB2SIDH, .data = MCP2515_LOAD_TXB2D0, .rts = MCP2515_RTS_TXB2 },
};

static const Mcp2515RxBuffer s_rx_buffers[] = {
  { .id = MCP2515_READ_RXB0SIDH, .data = MCP2515_READ_RXB0D0, .int_flag = MCP2515_CANINT_RX0IE },
  { .id = MCP2515_READ_RXB1SIDH, .data = MCP2515_READ_RXB1D0, .int_flag = MCP2515_CANINT_RX1IE },
};

// SPI commands - See Table 12-1
static void prv_reset(Mcp2515Storage *storage) {
  uint8_t payload[] = { MCP2515_CMD_RESET };
  spi_exchange(storage->spi_port, payload, sizeof(payload), NULL, 0);

  delay_us(100);
}

static void prv_read(Mcp2515Storage *storage, uint8_t addr, uint8_t *read_data, size_t read_len) {
  CRITICAL_SECTION_AUTOEND;
  uint8_t payload[] = { MCP2515_CMD_READ, addr };
  spi_exchange(storage->spi_port, payload, sizeof(payload), read_data, read_len);
}

static void prv_write(Mcp2515Storage *storage, uint8_t addr, uint8_t *write_data,
                      size_t write_len) {
  CRITICAL_SECTION_AUTOEND;
  uint8_t payload[MCP2515_MAX_WRITE_BUFFER_LEN];
  payload[0] = MCP2515_CMD_WRITE;
  payload[1] = addr;
  memcpy(&payload[2], write_data, write_len);
  spi_exchange(storage->spi_port, payload, sizeof(payload), write_data, write_len);
}

// See 12.10: *addr = (data & mask) | (*addr & ~mask)
static void prv_bit_modify(Mcp2515Storage *storage, uint8_t addr, uint8_t mask, uint8_t data) {
  CRITICAL_SECTION_AUTOEND;
  uint8_t payload[] = { MCP2515_CMD_BIT_MODIFY, addr, mask, data };
  spi_exchange(storage->spi_port, payload, sizeof(payload), NULL, 0);
}

static uint8_t prv_read_status(Mcp2515Storage *storage) {
  uint8_t payload[] = { MCP2515_CMD_READ_STATUS };
  uint8_t read_data[1] = { 0 };
  spi_exchange(storage->spi_port, payload, sizeof(payload), read_data, sizeof(read_data));

  return read_data[0];
}

static void prv_handle_rx(Mcp2515Storage *storage, uint8_t int_flags) {
  for (size_t i = 0; i < SIZEOF_ARRAY(s_rx_buffers); i++) {
    Mcp2515RxBuffer *rx_buf = &s_rx_buffers[i];
    if (int_flags & rx_buf->int_flag) {
      // Message RX

      // Read ID
      uint8_t id_payload[] = { MCP2515_CMD_READ_RX | rx_buf->id };
      Mcp2515IdRegs read_id_regs = { 0 };
      spi_exchange(storage->spi_port, id_payload, sizeof(id_payload), (uint8_t *)&read_id_regs,
                   sizeof(read_id_regs));
      Mcp2515Id id = {
        .sid_0_2 = read_id_regs.sidl.sid_0_2,
        .sidh = read_id_regs.sidh,
        .eid0 = read_id_regs.eid0,
        .eid8 = read_id_regs.eid8,
        .eid_16_17 = read_id_regs.sidl.eid_16_17,
      };
      bool extended = read_id_regs.sidl.ide;
      size_t dlc = read_id_regs.dlc.dlc;

      if (!extended) {
        // Standard IDs have garbage in the extended fields
        id.raw &= 0x7FF;
      }

      uint8_t data_payload[] = { MCP2515_CMD_READ_RX | rx_buf->data };
      uint64_t read_data = 0;
      spi_exchange(storage->spi_port, data_payload, sizeof(data_payload), (uint8_t *)&read_data,
                   sizeof(read_data));

      if (storage->rx_cb != NULL) {
        storage->rx_cb(id.raw, extended, read_data, dlc, storage->context);
      }
    }
  }
}

static void prv_handle_error(Mcp2515Storage *storage, uint8_t int_flags, uint8_t err_flags) {
  // Clear flags
  if (int_flags & MCP2515_CANINT_EFLAG) {
    // Clear error flag
    LOG_DEBUG("MCP2515_CANINT_EFLAG error\n");
    prv_bit_modify(storage, MCP2515_CTRL_REG_CANINTF, MCP2515_CANINT_EFLAG, 0);
  }

  if (err_flags & (MCP2515_EFLG_RX0_OVERFLOW | MCP2515_EFLG_RX1_OVERFLOW)) {
    // RX overflow - clear error flags
    uint8_t clear = 0;
    LOG_DEBUG("MCP2515_EFLG_RX0_OVERFLOW | MCP2515_EFLG_RX1_OVERFLOW error\n");
    prv_write(storage, MCP2515_CTRL_REG_EFLG, &clear, 1);
  }

  storage->errors.eflg = err_flags;
  prv_read(storage, MCP2515_CTRL_REG_TEC, &storage->errors.tec, 1);
  prv_read(storage, MCP2515_CTRL_REG_REC, &storage->errors.rec, 1);

  if (err_flags & MCP2515_EFLG_TX_BUS_OFF) {
    // Bus off - attempt to recover by resetting the chip?
    LOG_DEBUG("MCP2515_EFLG_TX_BUS_OFF error\n");
    if (storage->bus_err_cb != NULL) {
      storage->bus_err_cb(&storage->errors, storage->context);
    }
  }
}

static void prv_handle_int(const GpioAddress *address, void *context) {
  bool disabled = critical_section_start();
  Mcp2515Storage *storage = context;

  // Read CANINTF and EFLG
  struct {
    uint8_t canintf;
    uint8_t eflg;
  } regs;
  prv_read(storage, MCP2515_CTRL_REG_CANINTF, (uint8_t *)&regs, sizeof(regs));
  // Mask out flags we don't care about
  regs.canintf &= MCP2515_CANINT_EFLAG | MCP2515_CANINT_RX0IE | MCP2515_CANINT_RX1IE;

  // Either RX or error
  prv_handle_rx(storage, regs.canintf);
  prv_handle_error(storage, regs.canintf, regs.eflg);
  critical_section_end(disabled);
}

StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings) {
  storage->spi_port = settings->spi_port;
  storage->rx_cb = settings->rx_cb;
  storage->bus_err_cb = settings->bus_err_cb;
  storage->context = settings->context;
  storage->int_pin = settings->int_pin;

  const SpiSettings spi_settings = {
    .baudrate = settings->spi_baudrate,
    .mode = SPI_MODE_0,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
  };
  status_ok_or_return(spi_init(settings->spi_port, &spi_settings));

  prv_reset(storage);

  // Set to Config mode, CLKOUT /4
  prv_bit_modify(storage, MCP2515_CTRL_REG_CANCTRL,
                 MCP2515_CANCTRL_OPMODE_MASK | MCP2515_CANCTRL_CLKOUT_MASK,
                 MCP2515_CANCTRL_OPMODE_CONFIG | MCP2515_CANCTRL_CLKOUT_CLKPRE_4);

  // In order:
  // CNF3: PS2 Length = 6
  // CNF2: PS1 Length = 8, PRSEG Length = 1
  // CNF1: BRP = 1 (500kbps), 2 (250kbps), 3 (500kbps)
  // CANINTE: Enable error and receive interrupts
  // CANINTF: clear all IRQ flags
  // EFLG: clear all error flags
  const uint8_t registers[] = {
    0x05, MCP2515_CNF2_BTLMODE_CNF3 | MCP2515_CNF2_SAMPLE_3X | (0x07 << 3),
    settings->can_bitrate, MCP2515_CANINT_EFLAG | MCP2515_CANINT_RX1IE | MCP2515_CANINT_RX0IE,
    0x00, 0x00,
  };

  for (int i = 0; i < 6; i++) {
    LOG_DEBUG("Register: %d, %x\n", i, registers[i]); 
  }

  prv_write(storage, MCP2515_CTRL_REG_CNF3, registers, SIZEOF_ARRAY(registers));

  // Leave config mode
  uint8_t opmode =
      (settings->loopback ? MCP2515_CANCTRL_OPMODE_LOOPBACK : MCP2515_CANCTRL_OPMODE_NORMAL);
  prv_bit_modify(storage, MCP2515_CTRL_REG_CANCTRL, MCP2515_CANCTRL_OPMODE_MASK, opmode);

  uint8_t readback[sizeof(registers)] = { 0 };
  prv_read(storage, MCP2515_CTRL_REG_CNF3, readback, SIZEOF_ARRAY(readback));
  for (size_t i = 0; i < SIZEOF_ARRAY(readback); i++) {
    LOG_DEBUG("%d: read 0x%x\n", i, readback[i]);
  }

  // Active-low interrupt pin
  const GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
  };
  return gpio_init_pin(&settings->int_pin, &gpio_settings);
}

StatusCode mcp2515_register_cbs(Mcp2515Storage *storage, Mcp2515RxCb rx_cb, Mcp2515BusErrorCb bus_err_cb, void *context) {
  bool disabled = critical_section_start();
  storage->rx_cb = rx_cb;
  storage->bus_err_cb = bus_err_cb;
  storage->context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode mcp2515_tx(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                      size_t dlc) {
  CRITICAL_SECTION_AUTOEND;

  printf("%i\n", (int)5);
  // Get free transmit buffer
  uint8_t tx_status =
      __builtin_ffs(~prv_read_status(storage) &
                    (MCP2515_STATUS_TX0REQ | MCP2515_STATUS_TX1REQ | MCP2515_STATUS_TX2REQ));
  if (tx_status == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  // Status format: 0b01010100 = all TXxREQ bits set
  // ffs returns 1-indexed: (x-3)/2 -> 0b00000111 = all TXxREQ bits set
  Mcp2515TxBuffer *tx_buf = &s_tx_buffers[(tx_status - 3) / 2];

  Mcp2515Id tx_id = { .raw = id };
  Mcp2515IdRegs tx_id_regs = {
    .sidh = tx_id.sidh,
    .sidl = { .eid_16_17 = tx_id.eid_16_17,
              .ide = extended,
              .srr = false,
              .sid_0_2 = tx_id.sid_0_2 },
    .eid0 = tx_id.eid0,
    .eid8 = tx_id.eid8,
    .dlc = { .dlc = dlc, .rtr = false },
  };

  // Load ID: SIDH, SIDL, EID8, EID0, RTSnDLC
  uint8_t id_payload[] = {
    MCP2515_CMD_LOAD_TX | tx_buf->id,
    tx_id_regs.sidh,
    tx_id_regs.sidl.raw,
    tx_id_regs.eid0,
    tx_id_regs.eid8,
    tx_id_regs.dlc.raw,
  };
  spi_exchange(storage->spi_port, id_payload, sizeof(id_payload), NULL, 0);

  // Load data
  Mcp2515LoadTxPayload data_payload = {
    .cmd = MCP2515_CMD_LOAD_TX | tx_buf->data,
    .data = data,
  };
  spi_exchange(storage->spi_port, (uint8_t *)&data_payload, sizeof(data_payload), NULL, 0);

  // Send message
  uint8_t send_payload[] = { MCP2515_CMD_RTS | tx_buf->rts };
  spi_exchange(storage->spi_port, send_payload, sizeof(send_payload), NULL, 0);

  return STATUS_CODE_OK;
}

uint8_t counter3 = 1;
uint64_t counter4 = 0;
const uint8_t registers[] = { MCP2515_CTRL_REG_BFPCTRL,    // RX pins disabled by default
                              MCP2515_CTRL_REG_TXRTSCTRL,  // TX pins input by default
                              MCP2515_CTRL_REG_CANSTAT,
                              MCP2515_CTRL_REG_CANCTRL,
                              MCP2515_CTRL_REG_TEC,
                              MCP2515_CTRL_REG_REC,
                              MCP2515_CTRL_REG_CNF3,
                              MCP2515_CTRL_REG_CNF2,
                              MCP2515_CTRL_REG_CNF1,
                              MCP2515_CTRL_REG_CANINTE,
                              MCP2515_CTRL_REG_CANINTF,
                              MCP2515_CTRL_REG_EFLG,
                              MCP2515_CTRL_REG_TXB0CTRL,
                              MCP2515_CTRL_REG_TXB1CTRL,
                              MCP2515_CTRL_REG_TXB2CTRL,
                              MCP2515_CTRL_REG_RXB0CTRL,
                              MCP2515_CTRL_REG_RXB1CTRL };
void mcp2515_watchdog(SoftTimerId timer_id, void *context) {
  Mcp2515Storage *storage = context;
  if (counter4 == 0) {
    prv_handle_int(&storage->int_pin, storage);
    LOG_DEBUG("Interrupt timeout\n");
    
    // Read the status of the Receive buffer registers
    for (size_t i = 0; i < SIZEOF_ARRAY(registers); i++) {
      uint8_t data = 0;
      prv_read(storage, registers[i], &data, 1);
      LOG_DEBUG("0x%x: read 0x%x\n", registers[i], data);
    }
  }

  
  counter4 = 0;
  if (!status_ok(soft_timer_start_seconds(15, mcp2515_watchdog, storage, NULL))) {
    LOG_DEBUG("Could not start MCP watchdog 2\n");
  }
}

void mcp2515_poll(Mcp2515Storage *storage) {
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&storage->int_pin, &state);

  if (state == GPIO_STATE_LOW) {
    prv_handle_int(&storage->int_pin, storage);
    counter3++;
    counter4++;
  }
  if (counter3 % 20 == 0) {
    counter3 = 1;
    debug_led_toggle_state(DEBUG_LED_RED);
  }
}