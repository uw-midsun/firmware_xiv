#pragma once
//Module for sending can messages
#include "can.h"
#include "can_fifo.h"
#include "can_ack.h"
#include "can_fsm.h"

//Initializes the CAN
StatusCode steering_can_init();

//Proccesses a CAN event and pops an item off the queue
StatusCode steering_can_process_event(Event *e);