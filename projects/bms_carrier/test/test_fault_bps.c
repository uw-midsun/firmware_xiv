#include <string.h>

#include "bms.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"

static BmsStorage s_bms_storage;

static bool s_relay_faulted = false;

StatusCode TEST_MOCK(relay_fault)(RelayStorage *storage) {
  s_relay_faulted = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  memset(&s_bms_storage, 0, sizeof(s_bms_storage));
  s_relay_faulted = false;
}

void teardown_test(void) {}

void test_fault(void) {
  TEST_ASSERT_OK(fault_bps_init(&s_bms_storage));
  fault_bps(EE_BPS_STATE_FAULT_AFE_CELL, false);
  TEST_ASSERT_TRUE(s_relay_faulted);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_AFE_CELL, s_bms_storage.bps_storage.fault_bitset);
  fault_bps(EE_BPS_STATE_FAULT_RELAY, false);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_AFE_CELL | EE_BPS_STATE_FAULT_RELAY,
                    s_bms_storage.bps_storage.fault_bitset);
}

void test_fault_clear(void) {
  TEST_ASSERT_OK(fault_bps_init(&s_bms_storage));
  fault_bps(EE_BPS_STATE_FAULT_AFE_CELL, false);
  fault_bps(EE_BPS_STATE_FAULT_RELAY, false);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_AFE_CELL | EE_BPS_STATE_FAULT_RELAY,
                    s_bms_storage.bps_storage.fault_bitset);
  fault_bps(EE_BPS_STATE_FAULT_RELAY, true);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_AFE_CELL, s_bms_storage.bps_storage.fault_bitset);
  fault_bps(EE_BPS_STATE_FAULT_AFE_CELL, true);
  TEST_ASSERT_EQUAL(EE_BPS_STATE_OK, s_bms_storage.bps_storage.fault_bitset);
}
