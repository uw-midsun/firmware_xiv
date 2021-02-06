#pragma once
#include "can.h"
#include "can_ack.h"

StatusCode ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                        uint16_t num_remaining, void *context);

StatusCode can_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

void can_transmit_A(SoftTimerId timer_id, void *context);

void can_transmit_B(SoftTimerId timer_id, void *context);

void write_A_message(void);

void write_B_message(void);
