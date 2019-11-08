#include "fifo.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_FIFO_BUFFER_LEN 11
#define TEST_FIFO_OFFSET 0x12

static Fifo s_fifo;
static uint16_t s_buffer[TEST_FIFO_BUFFER_LEN];

static void __attribute__((unused)) prv_dump_fifo(void) {
  printf("%u elements in use\n", (uint16_t)fifo_size(&s_fifo));
  for (size_t i = 0; i < s_fifo.max_elems; i++) {
    void *elem = (uint8_t *)s_fifo.buffer + i * s_fifo.elem_size;
    const char *head = (elem == s_fifo.head) ? "H" : "";
    const char *next = (elem == s_fifo.next) ? "T" : "";

    printf("%u: 0x%x %s%s\n", (uint16_t)i, s_buffer[i], head, next);
  }
}

void setup_test(void) {
  fifo_init(&s_fifo, s_buffer);
}

void teardown_test(void) {}

void test_fifo_basic(void) {
  uint16_t temp = 0;
  TEST_ASSERT_NOT_OK(fifo_peek(&s_fifo, &temp));
  TEST_ASSERT_NOT_OK(fifo_pop(&s_fifo, NULL));

  // Fill buffer
  for (uint16_t i = TEST_FIFO_OFFSET; i < TEST_FIFO_BUFFER_LEN + TEST_FIFO_OFFSET; i++) {
    TEST_ASSERT_OK(fifo_push(&s_fifo, &i));
  }
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN, fifo_size(&s_fifo));

  // Attempt to push into full FIFO
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, fifo_push(&s_fifo, &temp));

  // Peek at element
  TEST_ASSERT_OK(fifo_peek(&s_fifo, &temp));
  TEST_ASSERT_EQUAL(TEST_FIFO_OFFSET, temp);
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN, fifo_size(&s_fifo));

  // Pop first element from FIFO
  temp = 0;
  TEST_ASSERT_OK(fifo_pop(&s_fifo, &temp));
  TEST_ASSERT_EQUAL(TEST_FIFO_OFFSET, temp);
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN - 1, fifo_size(&s_fifo));

  // Push new elment into FIFO
  temp = 0x4321;
  TEST_ASSERT_OK(fifo_push(&s_fifo, &temp));

  while (fifo_size(&s_fifo) > 0) {
    uint16_t x = 0;
    TEST_ASSERT_OK(fifo_pop(&s_fifo, &x));
    if (fifo_size(&s_fifo) == 0) {
      TEST_ASSERT_EQUAL(temp, x);
    }
  }
}

void test_fifo_arr(void) {
  // Test for basic array functionality
  uint16_t send_arr[4] = { 0x12, 0x34, 0x56, 0x78 };
  uint16_t rx_arr[4] = { 0 };

  TEST_ASSERT_OK(fifo_push_arr(&s_fifo, send_arr, SIZEOF_ARRAY(send_arr)));
  TEST_ASSERT_OK(fifo_pop_arr(&s_fifo, rx_arr, SIZEOF_ARRAY(rx_arr)));

  for (size_t i = 0; i < SIZEOF_ARRAY(send_arr); i++) {
    TEST_ASSERT_EQUAL(send_arr[i], rx_arr[i]);
  }
}

void test_fifo_arr_wrap(void) {
  // Test for push/pop wrapping
  uint16_t send_arr[4] = { 0x12, 0x34, 0x56, 0x78 };
  uint16_t rx_arr[4] = { 0 };

  for (size_t i = 0; i < TEST_FIFO_BUFFER_LEN - 2; i++) {
    uint16_t x = 0xDEAD;
    TEST_ASSERT_OK(fifo_push(&s_fifo, &x));
  }

  TEST_ASSERT_OK(fifo_pop(&s_fifo, NULL));
  TEST_ASSERT_OK(fifo_pop(&s_fifo, NULL));

  TEST_ASSERT_OK(fifo_push_arr(&s_fifo, send_arr, SIZEOF_ARRAY(send_arr)));
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN, fifo_size(&s_fifo));

  TEST_ASSERT_OK(fifo_pop_arr(&s_fifo, NULL, TEST_FIFO_BUFFER_LEN - 4));
  TEST_ASSERT_OK(fifo_pop_arr(&s_fifo, rx_arr, SIZEOF_ARRAY(rx_arr)));

  for (size_t i = 0; i < SIZEOF_ARRAY(rx_arr); i++) {
    TEST_ASSERT_EQUAL(send_arr[i], rx_arr[i]);
  }
}

void test_fifo_arr_end(void) {
  // Make sure that the tail is set properly
  uint16_t send_arr[4] = { 0x12, 0x34, 0x56, 0x78 };
  uint16_t rx_arr[4] = { 0 };

  for (size_t i = 0; i < TEST_FIFO_BUFFER_LEN - SIZEOF_ARRAY(send_arr); i++) {
    uint16_t x = 0xDEAD;
    TEST_ASSERT_OK(fifo_push(&s_fifo, &x));
  }

  TEST_ASSERT_OK(fifo_push_arr(&s_fifo, send_arr, SIZEOF_ARRAY(send_arr)));
  TEST_ASSERT_EQUAL(TEST_FIFO_BUFFER_LEN, fifo_size(&s_fifo));

  TEST_ASSERT_OK(fifo_pop(&s_fifo, NULL));

  uint16_t temp = 0xDEAD;
  TEST_ASSERT_OK(fifo_push(&s_fifo, &temp));
  temp = 0;

  TEST_ASSERT_OK(fifo_pop(&s_fifo, &temp));
  TEST_ASSERT_EQUAL(0xDEAD, temp);
}
