#include "chip_id.h"

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static ChipId chip_Id_1 = { 0 };
static ChipId chip_Id_2 = { 0 };

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_chip_id(void) {
  const uint32_t half_second_ms = 500;
  chip_Id_1 = chip_id_get();
  delay_us(half_second_ms);
  chip_Id_2 = chip_id_get();
  for (int i = 0; i < 2; i++) {
    LOG_DEBUG("%u", chip_Id_1.id[i]);
  }
  TEST_ASSERT_EQUAL_INT32_ARRAY(chip_Id_1.id, chip_Id_2.id, 2);
}
