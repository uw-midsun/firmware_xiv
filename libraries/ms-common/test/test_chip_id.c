#include "chip_id.h"

#include <inttypes.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

// Verifies chip ID is consistant and prints chip ID for debugging
void test_chip_id(void) {
  ChipId chip_id_1 = { 0 };
  ChipId chip_id_2 = { 0 };
  const uint32_t delay_second_ms = 10;
  chip_id_1 = chip_id_get();
  delay_ms(delay_second_ms);
  chip_id_2 = chip_id_get();
  for (uint8_t i = 0; i < 3; i++) {
    LOG_DEBUG("0x%" PRIx32 "\n", chip_id_1.id[i]);
  }
  TEST_ASSERT_EQUAL_UINT32_ARRAY(chip_id_1.id, chip_id_2.id, SIZEOF_ARRAY(chip_id_1.id));
}
