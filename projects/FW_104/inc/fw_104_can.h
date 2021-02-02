#pragma once
#include "can.h"

StatusCode prv_can_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

void prv_can_transmit_A(SoftTimerId timer_id, void *context);

void prv_can_transmit_B(SoftTimerId timer_id, void *context);

void prv_write_A_message();

void prv_write_B_message();
