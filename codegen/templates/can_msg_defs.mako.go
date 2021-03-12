<%namespace name="helpers" file="/helpers/helpers.mako" /> \
<% from data import NUM_CAN_MESSAGES, NUM_CAN_DEVICES, parse_can_device_enum, parse_can_message_enum, parse_can_frames %> \
package canmsgdefs

// NOTE: This file is generated by codegen-tooling.
// Currently, this file is copy-pasted from codegen output
// to canmsgdefs in the telemetry server.

// TODO: Automate this process.

// CanDeviceID is a 16-bit integer for the source of a CAN message
type CanDeviceId uint16

// CanMsgID is a 32-bit integer for the type of CAN message
type CanMsgId uint32

// For setting the CAN device
const (
  <% can_devices = parse_can_device_enum() %> \
${helpers.generate_enum_go(can_devices, 'SystemCanDevice', 'CanDeviceID')}
)  

// For setting the CAN message ID
const (
  <% can_messages = parse_can_message_enum(options.filename) %> \
${helpers.generate_enum_go(can_messages, 'SystemCanMessage', 'CanMsgID')}
)