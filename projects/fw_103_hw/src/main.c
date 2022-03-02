#include <stdio.h>
#include <stdbool.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"
#include "gpio_it.h


/*
 * Create a project that reads an ADC converted reading from pin A6
 * whenever a button connected to pin B2 is pressed and logs the output.
 * The adc reading should trigger whenever the button is released,
 * not initially when the button is pressed.
 * Follow the same submission procedures as the firmware 102 homework,
 * except the suffix of the branch should be fw_103.

Hints:
assume the button is active-high

register an interrupt with the appropriate InterruptEdge settings (google this or ask someone / your lead)

 */

int main( void ) {
    gpio_it_init();
    gpio_init();
    adc_init( ADC_MODE_SINGLE );

    GpioAddress button_addr = {
            .port = GPIO_PORT_A,
            .pin = 6,
    };

    GpioSettings pot_settings = {
            .direction = GPIO_DIR_IN,
            .state = GPIO_STATE_LOW,
            .resistor = GPIO_RES_NONE,
            .alt_function = GPIO_ALTFN_ANALOG,
    };

    gpio_init_pin( &button_addr, &pot_settings );

    adc_set_channel_pin( button_addr, true );

    gpio_it_get_edge( &button_addr, /*interrupt edge?*/);
    gpio_it_register_interrupt( &button_addr,
                               pot_settings /*is this right?*/,
                               InterruptEdge edge,
                               /*GpioItCallback callback*/,
                               NULL );

    if ( gpio_it_register_interrupt( &button_addr ) == STATUS_CODE_OK )
      LOG_DEBUG( "button data: %d\n", button_data );

    while (true) {
      wait();
    }

    /*while( true ) {
      uint16_t button_data = 0;
      if (adc_read_raw_pin(button_addr, &button_data) == STATUS_CODE_OK)
        LOG_DEBUG("button data: %d\n", button_data);
      else
        LOG_DEBUG("Failed to read button data");
      delay_ms(200);
    }*/



}


