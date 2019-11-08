#pragma once
// Internal MCP2515 register definitions
// See http://ww1.microchip.com/downloads/en/DeviceDoc/20001801H.pdf

// SPI commands: Table 12-1
#define MCP2515_CMD_RESET 0xC0
#define MCP2515_CMD_READ 0x03
#define MCP2515_CMD_READ_RX 0x90
#define MCP2515_CMD_WRITE 0x02
#define MCP2515_CMD_LOAD_TX 0x40
#define MCP2515_CMD_RTS 0x80
#define MCP2515_CMD_READ_STATUS 0xA0
#define MCP2515_CMD_RX_STATUS 0xB0
#define MCP2515_CMD_BIT_MODIFY 0x05

// READ_RX Arguments: Figure 12-3
#define MCP2515_READ_RXB0SIDH 0x00  // ID
#define MCP2515_READ_RXB0D0 0x02    // Data
#define MCP2515_READ_RXB1SIDH 0x04
#define MCP2515_READ_RXB1D0 0x06

// LOAD_TX Arguments: Figure 12-5
#define MCP2515_LOAD_TXB0SIDH 0x00
#define MCP2515_LOAD_TXB0D0 0x01
#define MCP2515_LOAD_TXB1SIDH 0x02
#define MCP2515_LOAD_TXB1D0 0x03
#define MCP2515_LOAD_TXB2SIDH 0x04
#define MCP2515_LOAD_TXB2D0 0x05

// RTS Arguments: Table 12-1 (RTS)
#define MCP2515_RTS_TXB0 0x01
#define MCP2515_RTS_TXB1 0x02
#define MCP2515_RTS_TXB2 0x04

// READ_STATUS Response: Figure 12-8
#define MCP2515_STATUS_RX0IF 0x01
#define MCP2515_STATUS_RX1IF 0x02
#define MCP2515_STATUS_TX0REQ 0x04
#define MCP2515_STATUS_TX0IF 0x08
#define MCP2515_STATUS_TX1REQ 0x10
#define MCP2515_STATUS_TX1IF 0x20
#define MCP2515_STATUS_TX2REQ 0x40
#define MCP2515_STATUS_TX2IF 0x80

// Control Registers: Table 11-2
#define MCP2515_CTRL_REG_BFPCTRL 0x0C    // RX pins disabled by default
#define MCP2515_CTRL_REG_TXRTSCTRL 0x0D  // TX pins input by default
#define MCP2515_CTRL_REG_CANSTAT 0x0E
#define MCP2515_CTRL_REG_CANCTRL 0x0F
#define MCP2515_CTRL_REG_TEC 0x1C
#define MCP2515_CTRL_REG_REC 0x1D
#define MCP2515_CTRL_REG_CNF3 0x28
#define MCP2515_CTRL_REG_CNF2 0x29
#define MCP2515_CTRL_REG_CNF1 0x2A
#define MCP2515_CTRL_REG_CANINTE 0x2B
#define MCP2515_CTRL_REG_CANINTF 0x2C
#define MCP2515_CTRL_REG_EFLG 0x2D
#define MCP2515_CTRL_REG_TXB0CTRL 0x30
#define MCP2515_CTRL_REG_TXB1CTRL 0x40
#define MCP2515_CTRL_REG_TXB2CTRL 0x50
#define MCP2515_CTRL_REG_RXB0CTRL 0x60
#define MCP2515_CTRL_REG_RXB1CTRL 0x70

// Filter/Mask Registers: Table 11-1
#define MCP2515_REG_RXF0SIDH 0x00
#define MCP2515_REG_RXF1SIDH 0x04
#define MCP2515_REG_RXF2SIDH 0x08
#define MCP2515_REG_RXF3SIDH 0x10
#define MCP2515_REG_RXF4SIDH 0x14
#define MCP2515_REG_RXF5SIDH 0x18
#define MCP2515_REG_RXM0SIDH 0x20
#define MCP2515_REG_RXM1SIDH 0x24

// CANCTRL: Register 10-1
#define MCP2515_CANCTRL_OPMODE_MASK 0xE0
#define MCP2515_CANCTRL_OPMODE_NORMAL 0x00
#define MCP2515_CANCTRL_OPMODE_SLEEP 0x20
#define MCP2515_CANCTRL_OPMODE_LOOPBACK 0x40
#define MCP2515_CANCTRL_OPMODE_LISTEN 0x60
#define MCP2515_CANCTRL_OPMODE_CONFIG 0x80

#define MCP2515_CANCTRL_CLKOUT_MASK 0x07
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_1 0x04  // CLKEN is automatically enabled
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_2 0x05
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_4 0x06
#define MCP2515_CANCTRL_CLKOUT_CLKPRE_8 0x07

// CNF3: Register 5-3
#define MCP2515_CNF3_PHSEG2_MASK 0x07

// CNF2: Register 5-2
#define MCP2515_CNF2_BTLMODE_MASK 0x80
#define MCP2515_CNF2_BTLMODE_CNF3 0x80

#define MCP2515_CNF2_SAMPLE_MASK 0x40
#define MCP2515_CNF2_SAMPLE_3X 0x40

#define MCP2515_CNF2_PHSEG1_MASK 0x38
#define MCP2515_CNF2_PRSEG_MASK 0x07

// CNF1: Register 5-1
#define MCP2515_CNF1_BRP_MASK 0x3F

// CANINTE/INTF: Register 7-1/2
#define MCP2515_CANINT_MSG_ERROR 0x80
#define MCP2515_CANINT_WAKEUP 0x40
#define MCP2515_CANINT_EFLAG 0x20
#define MCP2515_CANINT_TX2IE 0x10
#define MCP2515_CANINT_TX1IE 0x08
#define MCP2515_CANINT_TX0IE 0x04
#define MCP2515_CANINT_RX1IE 0x02
#define MCP2515_CANINT_RX0IE 0x01

// EFLG: Register 6-3
#define MCP2515_EFLG_RX1_OVERFLOW 0x80
#define MCP2515_EFLG_RX0_OVERFLOW 0x40
#define MCP2515_EFLG_TX_BUS_OFF 0x20
#define MCP2515_EFLG_TX_EP_FLAG 0x10
#define MCP2515_EFLG_RX_EP_FLAG 0x08
#define MCP2515_EFLG_TX_WARNING 0x04
#define MCP2515_EFLG_RX_WARNING 0x02
#define MCP2515_EFLG_ERROR_WARNING 0x01

// TXBnDLC: Register 3-7
#define MCP2515_TXBNDLC_RTR_SHIFT 6
#define MCP2515_TXBNDLC_RTR_FRAME 0x40
#define MCP2515_TXBNDLC_DLC_MASK 0x0F

typedef struct Mcp2515LoadTxPayload {
  uint8_t cmd;
  uint64_t data;
} __attribute__((packed)) Mcp2515LoadTxPayload;

// TX/RX buffer ID registers - See Registers 3-3 to 3-7, 4-4 to 4-8
typedef struct Mcp2515IdRegs {
  uint8_t sidh;
  union {
    struct {
      uint8_t eid_16_17 : 2;
      uint8_t unimplemented : 1;
      uint8_t ide : 1;
      uint8_t srr : 1;
      uint8_t sid_0_2 : 3;
    };
    uint8_t raw;
  } sidl;
  uint8_t eid8;
  uint8_t eid0;
  union {
    struct {
      uint8_t dlc : 4;
      uint8_t reserved : 2;
      uint8_t rtr : 1;
      uint8_t unimplemented : 1;
    };
    uint8_t raw;
  } dlc;
} Mcp2515IdRegs;

typedef union Mcp2515Id {
  struct {
    uint32_t sid_0_2 : 3;
    uint32_t sidh : 8;
    uint32_t eid0 : 8;
    uint32_t eid8 : 8;
    uint32_t eid_16_17 : 2;
    uint32_t padding : 3;
  };
  uint32_t raw;
} Mcp2515Id;
