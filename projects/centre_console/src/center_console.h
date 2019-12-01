
#pragma once
#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "event_queue.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

// implement the fsm led for when the power button is checked???
typedef enum {
  EVENT_BTN_IDLE = 0,
  EVENT_BTN_PUSH,
} BUTTON_EVENT;

// CAN Events
typedef enum {
  EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER = 0,
  EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE,
  EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL,
  EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE,
  EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,
  EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,
  NUM_EE_CENTER_CONSOLE_DIGITAL_INPUTS,
} EECenterConsoleCanEvents;

typedef struct {
  GpioAddress btn_addr;
  EECenterConsoleCanEvents can_event;
}CenterConsoleInputLink;

typedef struct {
  CenterConsoleInputLink power_input;
  CenterConsoleInputLink drive_input;
  CenterConsoleInputLink neutral_input;
  CenterConsoleInputLink reverse_input;
  CenterConsoleInputLink hazards_input;
  CenterConsoleInputLink low_beam_input;
}CenterConsoleStorage;

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

typedef struct {
  CenterConsoleInputLink* input_link;
  Event* e;
}GpioItCallbackContext;

static CenterConsoleStorage s_cc_storage;

StatusCode initialize_center_console(CenterConsoleStorage* cc_storage);

void can_emit_msg();
