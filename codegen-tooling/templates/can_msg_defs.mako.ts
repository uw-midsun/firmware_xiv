<%namespace name="helpers" file="/helpers/helpers.mako" /> \
<% from data import NUM_CAN_MESSAGES, NUM_CAN_DEVICES, parse_can_device_enum, parse_can_message_enum %> \
// For setting the CAN device
export const enum CanDevice {
  <% can_devices = parse_can_device_enum() %> \
  ${helpers.generate_enum(can_devices, 'CAN_DEVICE')}
}

// For setting the CAN message ID
export const enum CanMessage {
  <% can_messages = parse_can_message_enum(options.filename) %> \
  ${helpers.generate_enum(can_messages, 'CAN_MESSAGE')}
}
