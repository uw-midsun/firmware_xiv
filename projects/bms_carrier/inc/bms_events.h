#pragma once

typedef enum {
  BMS_CAN_EVENT_RX = 0,
  BMS_CAN_EVENT_TX,
  BMS_CAN_EVENT_FAULT,
  NUM_BMS_CAN_EVENTS
} BmsCanEvent;
