#include "event_queue.h"
#include "fsm.h"
#include "gpio_it.h"
#include "drive_buttons.h"






static void prv_drive_registered(const GpioAddress *address, void *context){
      event_raise(EVENT_DRIVE_PRESSED, NULL);
  }

static void prv_neutral_registered(const GpioAddress *address, void *context){
      event_raise(EVENT_NEUTRAL_PRESSED, NULL);
  }

static void prv_reverse_registered(const GpioAddress *address, void *context){
      event_raise(EVENT_REVERSE_PRESSED, NULL);
  }

//sets up interrupts for each of the 3 drive buttons
StatusCode drive_buttons_init(){
  
    InterruptSettings drive_buttons_it = {
        .type = INTERRUPT_TYPE_INTERRUPT,
        .priority = INTERRUPT_PRIORITY_HIGH,
    };
    
    gpio_it_register_interrupt(&drive_button_address[DRIVE_BUTTON], &drive_buttons_it,
    INTERRUPT_EDGE_RISING_FALLING, prv_drive_registered, NULL);

    gpio_it_register_interrupt(&drive_button_address[NEUTRAL_BUTTON], &drive_buttons_it,
    INTERRUPT_EDGE_RISING_FALLING, prv_neutral_registered, NULL);

    gpio_it_register_interrupt(&drive_button_address[REVERSE_BUTTON], &drive_buttons_it,
    INTERRUPT_EDGE_RISING_FALLING, prv_reverse_registered, NULL);
    
}








