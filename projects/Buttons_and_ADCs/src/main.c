#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static void prv_button_interrupt_handler(const GpioAddress *adc_address, void *context){
    GpioAddress *adc_position = context;
    uint16_t adc = 0;
    adc_read_converted_pin(*adc_position, &adc);
    LOG_DEBUG("ADC reading is: %d\n", adc);
}

int main(void){
    interrupt_init();
    gpio_init();
    gpio_it_init();

    typedef enum { BUTTON_B2 = 0, NUM_BUTTONS } Button;

    typedef enum { ADC_A6 = 0, NUM_SENSORS } ADC;

    //Button address
    static GpioAddress button_addresses = {
        .port = GPIO_PORT_B, 
        .pin = 2,
        };
    
    //Button setting
    static GpioSettings button_setting= { 
        .direction = GPIO_DIR_IN,
        .state = GPIO_STATE_LOW,
        .alt_function = GPIO_ALTFN_NONE,
        .resistor = GPIO_RES_NONE,
        };

    //Adc address
    static GpioAddress adc_addresses = { 
        .port = GPIO_PORT_A, 
        .pin = 6,
        };

    //Adc setting
    static GpioSettings adc_setting = { 
        .direction = GPIO_DIR_IN,
        .state = GPIO_STATE_LOW,
        .alt_function = GPIO_ALTFN_ANALOG,
        .resistor = GPIO_RES_NONE 
        };

    static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       
    .priority = INTERRUPT_PRIORITY_NORMAL,  
    };

    gpio_init_pin(&adc_addresses, &adc_setting);
    adc_init(ADC_MODE_SINGLE);
    adc_set_channel_pin(adc_addresses, true);

    gpio_it_register_interrupt(&button_addresses, &s_interrupt_settings,
                            INTERRUPT_EDGE_RISING, prv_button_interrupt_handler,
                            &adc_addresses);
    while (true) {
        wait();
    }
    return 0;
}

