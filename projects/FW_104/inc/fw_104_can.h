#pragma once
#include "can.h"

StatusCode can_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

void can_transmit_A(SoftTimerId timer_id, void *context);

void can_transmit_B(SoftTimerId timer_id, void *context);

void write_A_message(void);

void write_B_message(void);
