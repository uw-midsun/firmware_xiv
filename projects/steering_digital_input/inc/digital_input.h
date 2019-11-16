//Module for recieving digital inputs from steering interface 
//
//Requires GPIO,Interrupts,Event Queue,Soft-timer

#include "event_queue.h"
#include "gpio.h"
#include "status.h"

//Digital inputs IDs
typedef enum {
  STEERING_DIGITAL_INPUT_ID_HORN = 0,
  STEERING_DIGITAL_INPUT_ID_RADIO_PPT,
  STEERING_DIGITAL_INPUT_ID_HIGH_BEAM_FWD,
  STEERING_DIGITAL_INPUT_ID_HIGH_BEAM_BACK,
  STEERING_DIGITAL_INPUT_ID_REGEN_BRAKE_TOGGLE,
  STEERING_DIGITAL_INPUT_ID_HEADLIGHTS_ON_OFF,
  STEERING_DIGITAL_INPUT_ID_CC_ON_OFF,
  STEERING_DIGITAL_DIGITAL_INPUT_ID_CC_INCREASE_SPEED,
  STEERING_DIGITAL_DIGITAL_INPUT_ID_Cc_DECREASE_SPEED,
  NUM_STEERING_DIGITAL_INPUT_IDS,
} SteeringInterfaceDigitalInput;

//CAN IDs
typedef enum {
  STEERING_DIGITAL_INPUT_CAN_TX=0,
  STEERING_DIGITAL_INPUT_CAN_RX,
} SteeringInterfaceCANIDs;

// Digital Inputs
typedef struct {
  // CAN Events that will be raised
  EventId can_event[NUM_GPIO_STATES];
  //Pin for the input
  GpioAddress pin;
} SteeringDigitalInputCanEvents;

StatusCode steering_digital_input_init();