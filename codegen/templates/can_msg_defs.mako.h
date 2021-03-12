<%namespace name="helpers" file="/helpers/helpers.mako" /> \
<% from data import NUM_CAN_MESSAGES, NUM_CAN_DEVICES, parse_can_device_enum, parse_can_message_enum, parse_can_frames %> \
#pragma once

#include <stdbool.h>

#include "can_msg.h"

// For setting the CAN device
typedef enum {
  <% can_devices = parse_can_device_enum() %> \
      ${helpers.generate_enum(can_devices, 'SYSTEM_CAN_DEVICE')}
} SystemCanDevice;

// For setting the CAN message ID
typedef enum {
  <% can_messages = parse_can_message_enum(options.filename) %> \
      ${helpers.generate_enum(can_messages, 'SYSTEM_CAN_MESSAGE')}
} SystemCanMessage;
