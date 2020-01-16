
typedef enum {
  FAULT,
  NEUTRAL,
  DRIVE,
  NUM_CAR_EVENTS,
} InputCarEvents;

typedef enum {
  PRESSED = NUM_CAR_EVENTS + 1,
  RELEASED,
  NUM_BRAKE_EVENTS,
} BrakeEvents;

typedef enum {
  PEDAL_EVENT_THROTTLE_READING = NUM_BRAKE_EVENTS + 1,
  PEDAL_EVENT_THROTTLE_ENABLE,
  PEDAL_EVENT_THROTTLE_DISABLE,
  NUM_THROTTLE_EVENTS,
} ThrottleEvents;

// typedef enum {
// 	CAN_BRAKE_PRESSED = 0,
// 	CAN_BRAKE_RELEASED,
// 	NUM_BRAKE_CAN_EVENTS,
// } BrakeCanEvents;

// typedef enum {
// 	CAN_RX = NUM_BRAKE_CAN_EVENTS + 1,
// 	CAN_TX,
// 	CAN_FAULT,
// 	NUM_CAN_EVENTS,
// } CanEvents;

// typedef enum {
// 	CAR_INPUT_FAULT = NUM_CAN_EVENTS + 1,
// 	CAR_INPUT_NEUTRAL,
// 	CAR_INPUT_DRIVE,
// 	NUM_CAR_EVENTS,
// } InputCarEvents;

// typedef enum {
// 	BRAKE_PRESSED = NUM_CAR_EVENTS + 1,
// 	BRAKE_RELEASED,
// 	NUM_BRAKE_EVENTS,
// } BrakeEvents;

// typedef enum {
// 	PEDAL_EVENT_THROTTLE_READING = NUM_BRAKE_EVENTS + 1,
// 	PEDAL_EVENT_THROTTLE_ENABLE,
// 	PEDAL_EVENT_THROTTLE_DISABLE, 
// 	NUM_THROTTLE_EVENTS,
// } ThrottleEvents;
