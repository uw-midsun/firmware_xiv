#include "can_hw.h"
#include <string.h>
#include "interrupt.h"
#include "log.h"
#include "stm32f0xx.h"

#define CAN_HW_BASE CAN
#define CAN_HW_NUM_FILTER_BANKS 14

typedef struct CanHwTiming {
  uint16_t prescaler;
  uint8_t bs1;
  uint8_t bs2;
} CanHwTiming;

typedef struct CanHwEventHandler {
  CanHwEventHandlerCb callback;
  void *context;
} CanHwEventHandler;

// Generated settings using http://www.bittiming.can-wiki.info/
// Note that the BS1/BS2 register values are used +1, so we need to subtract 1
// from the calculated value to compenstate. The same is true for the prescaler,
// but the library subtracts 1 internally. The total time quanta is thus (BS1 +
// 1) + (BS2 + 1) + SJW (1) ~= 16 tq.
static CanHwTiming s_timing[NUM_CAN_HW_BITRATES] = {  // For 48MHz clock
  [CAN_HW_BITRATE_125KBPS] = { .prescaler = 24, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_250KBPS] = { .prescaler = 12, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_500KBPS] = { .prescaler = 6, .bs1 = 12, .bs2 = 1 },
  [CAN_HW_BITRATE_1000KBPS] = { .prescaler = 3, .bs1 = 12, .bs2 = 1 }
};
static CanHwEventHandler s_handlers[NUM_CAN_HW_EVENTS];
static uint8_t s_num_filters;

static void prv_add_filter(uint8_t filter_num, uint32_t mask, uint32_t filter) {
  CAN_FilterInitTypeDef filter_cfg = {
    .CAN_FilterNumber = filter_num,
    .CAN_FilterMode = CAN_FilterMode_IdMask,
    .CAN_FilterScale = CAN_FilterScale_32bit,
    .CAN_FilterIdHigh = filter >> 16,
    .CAN_FilterIdLow = filter,
    .CAN_FilterMaskIdHigh = mask >> 16,
    .CAN_FilterMaskIdLow = mask,
    .CAN_FilterFIFOAssignment = (filter_num % 2),
    .CAN_FilterActivation = ENABLE,
  };

  CAN_FilterInit(&filter_cfg);
}

StatusCode can_hw_init(const CanHwSettings *settings) {
  memset(s_handlers, 0, sizeof(s_handlers));
  s_num_filters = 0;

  GpioSettings gpio_settings = {
    .alt_function = GPIO_ALTFN_4,  //
    .direction = GPIO_DIR_OUT,     //
  };
  gpio_init_pin(&settings->tx, &gpio_settings);
  gpio_settings.direction = GPIO_DIR_IN;
  gpio_init_pin(&settings->rx, &gpio_settings);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);

  CAN_DeInit(CAN_HW_BASE);

  CAN_InitTypeDef can_cfg;
  CAN_StructInit(&can_cfg);

  can_cfg.CAN_Mode = settings->loopback ? CAN_Mode_Silent_LoopBack : CAN_Mode_Normal;
  can_cfg.CAN_SJW = CAN_SJW_1tq;
  can_cfg.CAN_ABOM = ENABLE;
  can_cfg.CAN_BS1 = s_timing[settings->bitrate].bs1;
  can_cfg.CAN_BS2 = s_timing[settings->bitrate].bs2;
  can_cfg.CAN_Prescaler = s_timing[settings->bitrate].prescaler;
  CAN_Init(CAN_HW_BASE, &can_cfg);

  CAN_ITConfig(CAN_HW_BASE, CAN_IT_TME, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_FMP0, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_FMP1, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_ERR, ENABLE);
  stm32f0xx_interrupt_nvic_enable(CEC_CAN_IRQn, INTERRUPT_PRIORITY_HIGH);

  // Allow all messages by default, but reset the filter count so it's
  // overwritten on the first filter
  prv_add_filter(0, 0, 0);
  s_num_filters = 0;

  return STATUS_CODE_OK;
}

StatusCode can_hw_register_callback(CanHwEvent event, CanHwEventHandlerCb callback, void *context) {
  if (event >= NUM_CAN_HW_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_handlers[event] = (CanHwEventHandler){
    .callback = callback,  //
    .context = context,    //
  };

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter(uint32_t mask, uint32_t filter, bool extended) {
  if (s_num_filters >= CAN_HW_NUM_FILTER_BANKS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filter banks.");
  }

  // 32-bit Filter - Identifer Mask
  // STID[10:3] | STID[2:0] EXID[17:13] | EXID[12:5] | EXID[4:0] [IDE] [RTR] 0
  size_t offset = extended ? 3 : 21;
  // We always set the IDE bit for the mask so we distinguish between standard
  // and extended
  uint32_t mask_val = (mask << offset) | (1 << 2);
  uint32_t filter_val = (filter << offset) | ((uint32_t)extended << 2);

  prv_add_filter(s_num_filters, mask_val, filter_val);
  s_num_filters++;
  return STATUS_CODE_OK;
}

CanHwBusStatus can_hw_bus_status(void) {
  if (CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_BOF) == SET) {
    return CAN_HW_BUS_STATUS_OFF;
  } else if (CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_EWG) == SET ||
             CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_EPV) == SET) {
    return CAN_HW_BUS_STATUS_ERROR;
  }

  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint32_t id, bool extended, const uint8_t *data, size_t len) {
  // We can set both since the used ID is determined by tx_msg.IDE
  CanTxMsg tx_msg = {
    .StdId = id,                                          //
    .ExtId = id,                                          //
    .IDE = extended ? CAN_Id_Extended : CAN_Id_Standard,  //
    .DLC = len,                                           //
  };

  memcpy(tx_msg.Data, data, len);

  uint8_t tx_mailbox = CAN_Transmit(CAN_HW_BASE, &tx_msg);
  if (tx_mailbox == CAN_TxStatus_NoMailBox) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW TX failed");
  }

  return STATUS_CODE_OK;
}

bool can_hw_receive(uint32_t *id, bool *extended, uint64_t *data, size_t *len) {
  // 0: No messages available
  // 1: FIFO0 has received a message
  // 2: FIFO1 has received a message
  // 3: Both have received messages: arbitrarily pick FIFO0
  uint8_t fifo_status = (CAN_MessagePending(CAN_HW_BASE, CAN_FIFO0) != 0) |
                        (CAN_MessagePending(CAN_HW_BASE, CAN_FIFO1) != 0) << 1;
  uint8_t fifo = (fifo_status == 2);

  if (fifo_status == 0) {
    // No messages available
    return false;
  }

  CanRxMsg rx_msg = { 0 };
  CAN_Receive(CAN_HW_BASE, fifo, &rx_msg);

  *extended = (rx_msg.IDE == CAN_Id_Extended);
  *id = *extended ? rx_msg.ExtId : rx_msg.StdId;
  *len = rx_msg.DLC;
  memcpy(data, rx_msg.Data, sizeof(*rx_msg.Data) * rx_msg.DLC);

  return true;
}

void CEC_CAN_IRQHandler(void) {
  bool run_cb[NUM_CAN_HW_EVENTS] = {
    [CAN_HW_EVENT_TX_READY] = CAN_GetITStatus(CAN_HW_BASE, CAN_IT_TME) == SET,
    [CAN_HW_EVENT_MSG_RX] = CAN_GetITStatus(CAN_HW_BASE, CAN_IT_FMP0) == SET ||
                            CAN_GetITStatus(CAN_HW_BASE, CAN_IT_FMP1) == SET,
    [CAN_HW_EVENT_BUS_ERROR] = CAN_GetITStatus(CAN_HW_BASE, CAN_IT_ERR) == SET,
  };

  for (int event = 0; event < NUM_CAN_HW_EVENTS; event++) {
    CanHwEventHandler *handler = &s_handlers[event];
    if (handler->callback != NULL && run_cb[event]) {
      handler->callback(handler->context);
      break;
    }
  }

  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_ERR);
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_TME);
}
