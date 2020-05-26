#pragma once
// Module to process events on the queue and send CAN messages
// Requires GPIO,Interrupts,Event Queue, and CAN to be initialized
#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "status.h"

#define STEERING_CAN_DEVICE_ID 0x1

// Receives an event and sends a CAN message
StatusCode steering_can_process_event(Event *e);
