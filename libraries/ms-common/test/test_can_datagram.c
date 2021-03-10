#include "can_datagram.h"

#include <stdint.h>
#include <stdio.h>

#include "status.h"
#include "test_helpers.h"
#include "unity.h"
#include "log.h"
#include "event_queue.h"

static uint8_t s_data[24];
static uint8_t s_dst[8];

void setup_test(void) {
    event_queue_init();
}

void teardown_test(void) {}

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool start_message) {
    LOG_DEBUG("TX CALLBACK\n");
    return STATUS_CODE_OK;
}


void test_can_datagram(void) {
    CanDatagramSettings settings = {
        .tx_cb = prv_tx_callback,
        .mode = CAN_DATAGRAM_MODE_TX,
    };
    can_datagram_init(&settings);
    can_datagram_start_tx();
    can_datagram_set_address_buffer(s_dst, 8);
    can_datagram_set_data_buffer(s_data, 24);

    Event e = { 0 };
    while(!can_datagram_tx_complete()) {
      while (event_process(&e) != STATUS_CODE_OK) {}
        can_datagram_process_event(&e);
    }
}