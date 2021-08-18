#include "wait.h"     
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"


typedef enum { BUTTON_B2 = 0, NUM_BUTTONS } Button;

typedef enum { SENSOR_A6 = 0, NUM_SENSORS } Sensor;

static GpioAddress button_addresses[] = {
    [BUTTON_B2] = { .port = GPIO_PORT_B, .pin = 2}
    };

static GpioAddress sensor_addresses[] = { 
    [SENSOR_A6] = { .port = GPIO_PORT_A, .pin = 6} 
    };

static GpioSettings button_setting= { 
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
    };

static GpioSettings sensor_setting = { 
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE 
    };

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       
  .priority = INTERRUPT_PRIORITY_NORMAL,  
    };

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  GpioAddress *button_address = (GpioAddress *)context;
  gpio_toggle_state(button_address);
}

