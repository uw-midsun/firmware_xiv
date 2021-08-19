#include "wait.h"     
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

static void prv_adc_reading(const GpioAddress *address, void *context){
    GpioAddress *address = context;
    uint16_t adc = 0;
    adc_read_converted_pin(*address, &adc);
    LOG_DEBUG("ADC reading: %d\n", adc);
}

int main(void){
    interrupt_init();
    gpio_init();
    gpio_it();

    typedef enum { BUTTON_B2 = 0, NUM_BUTTONS } Button;

    typedef enum { ADC_A6 = 0, NUM_SENSORS } ADC;

    static GpioAddress button_addresses[] = {
        [BUTTON_B2] = { .port = GPIO_PORT_B, .pin = 2}
        };
        
    static GpioSettings button_setting= { 
        .direction = GPIO_DIR_IN,
        .state = GPIO_STATE_LOW,
        .alt_function = GPIO_ALTFN_NONE,
        .resistor = GPIO_RES_NONE,
        };

    static GpioAddress sensor_addresses[] = { 
        [ADC_A6] = { .port = GPIO_PORT_A, .pin = 6} 
        };

    static GpioSettings adc_setting = { 
        .direction = GPIO_DIR_IN,
        .state = GPIO_STATE_LOW,
        .alt_function = GPIO_ALTFN_NONE,
        .resistor = GPIO_RES_NONE 
        };

    static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       
    .priority = INTERRUPT_PRIORITY_NORMAL,  
        };
    
    //gpio_init_pin(&potentiometer_addr, &pot_settings);
    //adc_set_channel_pin(potentiometer_addr, true);
    //adc_init(ADC_MODE_SINGLE);
    //gpio_it_register_interrupt(&button_addresses[BUTTON_GREEN], &s_interrupt_settings,
    //                        INTERRUPT_EDGE_RISING, prv_button_interrupt_handler,
    //                        &led_addresses[LED_BLUE]);
    while (true) {
        wait();
    }
}