#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"


// GPIO address that we will allow us to read button data
const GpioAddress potentiometer_addr_A6 = { // could be passed as context
        .port = GPIO_PORT_A,
        .pin = 6,
};


// function that is called each time the button is clicked
static void prv_button_interrupt_callback(const GpioAddress *potentiometer_addr_B2, void *context) {

    uint16_t data = 0;
    StatusCode button_data = adc_read_raw_pin( potentiometer_addr_A6, &data);  // see what this takes in in the header file

    LOG_DEBUG("Status Code: %d\n", button_data);
    LOG_DEBUG("button data: %d\n", data);
};

int main(void) {
    interrupt_init();
    gpio_init();
    gpio_it_init();
    soft_timer_init();
    adc_init(ADC_MODE_SINGLE);

    // GPIO Address where button is pressed
    GpioAddress potentiometer_addr_B2 = {
            .port = GPIO_PORT_B,
            .pin = 2,
    };

    GpioSettings pot_settings = {
            .direction = GPIO_DIR_IN,
            .state = GPIO_STATE_LOW,
            .resistor = GPIO_RES_NONE,
            .alt_function = GPIO_ALTFN_ANALOG,
    };

    gpio_init_pin(&potentiometer_addr_A6, &pot_settings);
    gpio_init_pin(&potentiometer_addr_B2, &pot_settings);

    InterruptSettings button_interrupt_settings = {
            .type = INTERRUPT_TYPE_INTERRUPT,
            .priority = INTERRUPT_PRIORITY_HIGH,
    };

    // allow A6 to read adc
    adc_set_channel_pin(potentiometer_addr_A6, true);

    // registering interrupt for the B2 GPIO Address
    gpio_it_register_interrupt(&potentiometer_addr_B2, &button_interrupt_settings,
                               INTERRUPT_EDGE_RISING, prv_button_interrupt_callback, NULL);

    while (true) {
        gpio_it_trigger_interrupt(&potentiometer_addr_B2);
        delay_ms(200);
    }
}
