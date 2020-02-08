#pragma once
// Module to sets up all the interrupts for the GPIO pins to raise events
// in the event queue when triggered
// Requires GPIO,Interrupts,Event Queue,Soft-timer, CAN
#include "can.h"
#include "can_fsm.h"
#include "can_msg.h"
#include "can_ack.h"
#include "can_fifo.h"
#include "can_hw.h"
#include "can_rx.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"

#define STEERING_CAN_DEVICE_ID 0x1

typedef enum {
    STEERING_CAN_FAULT = 0,
    STEERING_CAN_EVENT_RX,
    STEERING_CAN_EVENT_TX,
}SteeringCanEvents;

StatusCode steering_can_init();

//Receives an event and sends a CAN message
StatusCode steering_can_process_event(Event e);

