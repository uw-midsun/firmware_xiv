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
    [BUTTON_B2] = { .port = GPIO_PORT_B, .pin =  }
    };

static GpioAddress sensor_addresses[] = { 
    [SENSOR_A6] = { .port = GPIO_PORT_A, .pin =  } 
    };