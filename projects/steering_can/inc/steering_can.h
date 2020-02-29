#pragma once
// Module to process events on the queue and send CAN messages
// Requires GPIO,Interrupts,Event Queue, and CAN to be initialized
#include "can.h"
#include "can_ack.h"
#include "can_fifo.h"
#include "can_fsm.h"
#include "can_hw.h"
#include "can_msg.h"
#include "can_rx.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"

// Receives an event and sends a CAN message
StatusCode steering_can_process_event(Event e);
