#include "gpio.h"
#include "event_queue.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"

#include "charger_connection_sense.h"
#include "charger_events.h"

void setup_test(void) {
    event_queue_init();
    gpio_init();
    gpio_it_init();
    TEST_ASSERT_OK(connection_sense_init());
}

void teardown_test(void) {}