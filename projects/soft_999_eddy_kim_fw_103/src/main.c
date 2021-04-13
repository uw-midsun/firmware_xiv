#include "adc.h"
#include "gpio.h"
#include "log.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "soft_timer.h"
#include "wait.h"

static void callback(const GpioAddress *address, void *context) {
    // Is it right to pass the GpioAddress A6 by *context? I'm unsure what *address is supposed to return
    GpioAddress *A6 = context;
    uint16_t A6_data = 0;
    adc_read_raw_pin(*A6, &A6_data);
    LOG_DEBUG("%d\n", A6_data); 
    return;
} 

int main(void) {
    gpio_init();
    interrupt_init();

    // Unsure if these are defined properly
    GpioAddress B2 = { .port = GPIO_PORT_B, .pin = 2 };
    GpioAddress A6 = { .port = GPIO_PORT_A, .pin = 6 };

    // Unsure if these are defined properly as well
    GpioSettings B2_settings = {
        GPIO_DIR_IN,
        GPIO_STATE_LOW,
        GPIO_RES_NONE,
        GPIO_ALTFN_ANALOG
    };

    GpioSettings A6_settings = {
        GPIO_DIR_IN,
        GPIO_STATE_LOW,
        GPIO_RES_NONE,
        GPIO_ALTFN_ANALOG
    };

    adc_init(ADC_MODE_SINGLE);

    adc_set_channel_pin(A6, true);

    // Unsure if it's necessary to set channel pin for B2 as well
    adc_set_channel_pin(B2, true);

    InterruptSettings settings = {INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL};
    InterruptType edge = {INTERRUPT_EDGE_FALLING};

    gpio_it_register_interrupt(&B2, &settings, edge, callback, &A6);

    while (true) {
        wait();
    }

    return 0;
}
