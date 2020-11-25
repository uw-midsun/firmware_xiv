#include "i2c_write.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include <math.h>
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"

#define I2C1_SDA \
    {.port = GPIO_PORT_B, .pin = 9}
#define I2C1_SCL \
    {.port = GPIO_PORT_B, .pin = 8}
#define I2C2_SDA \
    {.port = GPIO_PORT_B, .pin = 11}
#define I2C2_SCL \
    {.port = GPIO_PORT_B, .pin = 10}

typedef enum {
    TEST_CAN_EVENT_TX = 0,
    TEST_CAN_EVENT_RX,
    TEST_CAN_EVENT_FAULT,
    NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;
static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static void *s_received_context;

static StatusCode s_status_return;

static StatusCode prv_rx_i2c_write_callback(uint8_t data[8], void *context, bool *tx_result) {
    LOG_DEBUG("CALLBACK!\n");
    s_times_callback_called++;
    memcpy(s_received_data, data, 8);
    s_received_context = context;
    *tx_result = false;
    return s_status_return;
}

void setup_test(void) {
    initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                    TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
    TEST_ASSERT_OK(dispatcher_init());
    TEST_ASSERT_OK(i2c_write_init());
    s_times_callback_called = 0;
    memset(s_received_data, 0, sizeof(s_received_data));
    s_status_return = STATUS_CODE_OK;

    TEST_ASSERT_OK(
        dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_rx_i2c_write_callback, NULL));
}

void teardown_test(void) {}

void test_writing_over_i2c(void) {
    uint8_t data[8] = {BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, I2C_PORT_1, 0x74, 7, 0, 0, 0, 0};
    uint8_t data_write[8] = {BABYDRIVER_MESSAGE_I2C_WRITE_DATA, 0, 0, 0, 0, 0, 0, 0};

    CAN_TRANSMIT_BABYDRIVER(data[0], data[1], data[2],
                            data[3], data[4], data[5], data[6], data[7]);
    CAN_TRANSMIT_BABYDRIVER(data_write[0], data_write[1], data_write[2], data_write[3],
                            data_write[4], data_write[5], data_write[6], data_write[7]);

    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);


    MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

    LOG_DEBUG("WORKS?\n");

    TEST_ASSERT_EQUAL(2, s_times_callback_called);
}
