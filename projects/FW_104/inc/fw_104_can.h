#pragma once
#include "can.h"

static StatusCode prv_can_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

static void prv_can_transmit(SoftTimerId timer_id, void *context);

static StatusCode prv_write_A_message();

static StatusCode prv_write_B_message();
