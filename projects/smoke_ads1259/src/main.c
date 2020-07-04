#include "ads1259_adc.h"
#include "log.h"
#include "delay.h"
#include "soft_timer.h"
#include "gpio.h"
#include "interrupt.h"

//slightly larger than conversion time of adc
#define CONVERSION_TIME_MS 18
#define READING_QUEUE_LENGTH 10
static Ads1259Storage s_storage;
static int s_index;

static void prv_rx_error_handler_cb(Ads1259StatusCode code, void *context) {
    if(code == ADS1259_STATUS_CODE_OUT_OF_RANGE) {
        LOG_DEBUG("ERROR: OUT OF RANGE\n");
    }
    else if(code == ADS1259_STATUS_CODE_CHECKSUM_FAULT) {
        LOG_DEBUG("ERROR: CHECKSUM FAULT\n");
    }
}

static void prv_dump_queue(double *queue) {
    LOG_DEBUG("\n\nDUMPING QUEUE....\n");
    for(int reading = 0; reading < READING_QUEUE_LENGTH; reading++) {
        LOG_DEBUG("VOLTAGE READING: %lf\n", queue[reading]);
        queue[reading] = 0;
    }
    s_index = 0;
}


static void prv_periodic_read(SoftTimerId id, void* context) {
    double *queue = context;
    if(s_index < 10) {
    ads1259_get_conversion_data(&s_storage);
    queue[s_index] = s_storage.reading;
    s_index++;
    soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, queue, NULL);
    } else {
        prv_dump_queue(queue);
    }
    
}

int main() {
    const Ads1259Settings settings = {
        .spi_port = SPI_PORT_2,
        .mosi = { .port = GPIO_PORT_B, 15 },
        .miso = { .port = GPIO_PORT_B, 14 },
        .sclk = { .port = GPIO_PORT_B, 13 },
        .cs = { .port = GPIO_PORT_B, 12 },
        .handler = prv_rx_error_handler_cb,
    };
    interrupt_init();
    gpio_init();
    soft_timer_init();
    ads1259_init(&settings, &s_storage);
    
    s_index = 0;
    double reading_queue[READING_QUEUE_LENGTH];
    
    
    
    while (1)
    {
        soft_timer_start_millis(CONVERSION_TIME_MS, prv_periodic_read, reading_queue, NULL);
        delay_ms(1000);
    }
    
}