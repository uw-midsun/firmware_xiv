#include "event_queue.h"
#include "fsm.h"
#include "drive_buttons.h"





static const GpioAddress drive_button_address[numStates] = {
    { .port = GPIO_PORT_B, .pin = 5 },
    { .port = GPIO_PORT_B, .pin = 4 },
    { .port = GPIO_PORT_B, .pin = 3 },
};


