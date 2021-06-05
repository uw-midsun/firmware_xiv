#pragma once
#include "can.h"
#include "can_ack.h"

StatusCode ACK_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                        uint16_t num_remaining, void *context);

StatusCode CAN_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

void CAN_A_send(SoftTimerId time_id, void *context);

void CAN_B_send(SoftTimerId time_id, void *context);

void write_CAN_messages(void);
