#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"
 /*
  *
  * Based on the MSXII pedal board schematics, write a function that writes
  * 0b0010010010100110 to the configuration register, then reads the conversion register.

Hint: you’ll need to look at the schematics for the pedal board and the datasheet for the ADC.

  *
  */

#define CAN_DEVICE_ID //something
#define CAN_BITRATE //something

 typedef enum {
   CAN_RX = 0,
   CAN_TX,
   CAN_FAULT,
   CAN_EVENTS,
 } CanEvent;

 typedef enum {
   MSG_TYPE_DATA = 0,
   MSG_TYPE_ACK,
   NUM_CAN_MSG_TYPES,
 } CanMsgType;

 typedef u_int16_t CanMessageId;

 can_register_rx_handler rx_handler ( message_id, /*handler?*/, *context) {

 }

 typedef struct CanMessage {
   u_int_16 source_id;
   CanMessageId msg_id; // this is just an int
   union {
     uint64_t data;
     uint32_t data_u32[2];
     uint16_t data_u16[4];
     uint8_t data_u8[8];
   };
   CanMsgType type;
   size_t dlc;
 } CanMessage;

 int main() {

   static *CanStorage s_can_storage = { 0 };

   *CanSettings can_settings = {
     .device_id = CAN_DEVICE_ID,
     .bitrate = CAN_BITRATE,
     .tx = { GPIO_PORT_A,  },
     .rx = { GPIO_PORT_A,  },
     .rx_event = CAN_RX,
     .tx_event = CAN_TX,
     .fault_event = CAN_FAULT,
   };

   // i have can settings so i can initialize it
   // // still need to define some values taken from the schematics??
   can_init( can_settings, s_can_storage );

   CanMessageId message_id = ;

   // message to send for the handler
   CanMessage message = {
     /* define this stuff */
   };

   can_register_rx_handler( message, handler, *context)

   return 0;
}