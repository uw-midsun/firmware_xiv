#include "bootloader_mcu.h"
#include "config.h"
#include "crc32.h"
#include "flash.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// A self defined test to check if memory is not equal
#define TEST_ASSERT_NOT_EQUAL_MEMORY(expected, actual, len) \
  prv_test_not_equal_memory((uint8_t *)(expected), (uint8_t *)(actual), (len))

// This is so the real persist_init can be used in the mock function
// You can't use __real_persist_init without telling the gcc its signiture
// hence the declaration
extern StatusCode __real_persist_init(PersistStorage *persist, FlashPage page, void *blob,
                                      size_t blob_size, bool overwrite);
// This a function pointer that will set itself to prv_change_config_page for
// test_config_commit_corruption to mimic a corruption in page 1
static void (*s_function_pointer_hook)(PersistStorage *persist, BootloaderConfig *config) = NULL;

static PersistStorage s_test_persist_storage_1 = { 0 };
static BootloaderConfig s_test_config_1 = { 0 };

static PersistStorage s_test_persist_storage_2 = { 0 };
static BootloaderConfig s_test_config_2 = { 0 };

// I need to create an input config to give config_commit();
// For test_config_commit_works();
// I use random values for the input_config
static BootloaderConfig test_input_config_works = { .crc32 = 1,
                                                    .controller_board_id = 1,
                                                    .controller_board_name = "a",
                                                    .project_present = true,
                                                    .project_name = "a",
                                                    .project_info = "a",
                                                    .git_version = "a",
                                                    .application_crc32 = 1,
                                                    .application_size = 1 };

// I need to create an input config to give config_commit();
// For test_config_commit_corruption();
// I use random values for the input_config
static BootloaderConfig test_input_config_corruption = { .crc32 = 1,
                                                         .controller_board_id = 2,
                                                         .controller_board_name = "b",
                                                         .project_present = false,
                                                         .project_name = "b",
                                                         .project_info = "b",
                                                         .git_version = "b",
                                                         .application_crc32 = 2,
                                                         .application_size = 2 };

// This sets a type flash page to the config pages from bootloader_mcu
#define BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_1_START))
#define BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_2_START))

static void prv_change_config_page(PersistStorage *persist, BootloaderConfig *config) {
  // This function changes the values in the input config
  // This is to mimic the event in which page 1 gets corrupted
  // during config_commit
  config->controller_board_id = 6;
  config->application_crc32 = 6;
  config->application_size = 6;
  persist_commit(persist);
}

static void prv_test_not_equal_memory(uint8_t *expected, uint8_t *actual, size_t len) {
  // This function is the implementation of the TEST_ASSERT_NOT_EQUAL_MEMORY
  bool equal_memory = true;
  for (size_t i = 0; i < len; i++) {
    if (!(expected[i] == actual[i])) equal_memory = false;
  }
  TEST_ASSERT_FALSE_MESSAGE(equal_memory, "Expected unequal memory");
}

void setup_test(void) {
  flash_init();
  interrupt_init();
  soft_timer_init();
  crc32_init();

  flash_erase(BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE);
  flash_erase(BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE);

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  // Input "test values" for the two config pages
  s_test_config_1.crc32 = 0;
  s_test_config_2.crc32 = 0;

  s_test_config_1.crc32 = crc32_arr((uint8_t *)&s_test_config_1, sizeof(BootloaderConfig));
  persist_commit(&s_test_persist_storage_1);
  s_test_config_2.crc32 = crc32_arr((uint8_t *)&s_test_config_2, sizeof(BootloaderConfig));
  persist_commit(&s_test_persist_storage_2);

  TEST_ASSERT_OK(config_init());
}

void test_default_config_changes(void) {
  // The test tests for the default config changes, the two flash pages are firstly cleared and then
  // config_init will be called  Then it will verify that the config obtained from config_get is the
  // default config and then test that the default config was correctly  recorded on flash by
  // calling config_init again with a config_get.
  flash_erase(BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE);
  flash_erase(BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE);

  config_init();

  BootloaderConfig test_input_config = { 0 };
  config_get(&test_input_config);
  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &test_input_config, sizeof(BootloaderConfig));

  config_init();
  config_get(&test_input_config);
  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &test_input_config, sizeof(BootloaderConfig));
}

void teardown_test(void) {}

void test_config_verify(void) {
  // Test 1, testing if config_verify works under normal circumstances
  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &s_test_config_2, sizeof(BootloaderConfig));

  // Test 2, testing if config_verify works for a faulty page (in this case, page 1)
  // Below, I'm purposefully changing config 1 so that it becomes faulty
  s_test_config_1.controller_board_id = 2;
  s_test_config_1.application_crc32 = 2;
  s_test_config_1.application_size = 2;
  persist_commit(&s_test_persist_storage_1);

  TEST_ASSERT_OK(config_init());

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &s_test_config_2, sizeof(BootloaderConfig));

  // Test 3, testing if config_verify works for a faulty page (in this case, page 2)
  // Below, I'm purposefully changing config 2 so that it becomes faulty
  s_test_config_2.controller_board_id = 3;
  s_test_config_2.application_crc32 = 3;
  s_test_config_2.application_size = 3;
  persist_commit(&s_test_persist_storage_2);

  TEST_ASSERT_OK(config_init());

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &s_test_config_2, sizeof(BootloaderConfig));

  // Test 4, testing if config_verify throws the correct error if both pages are corrupted
  s_test_config_1.controller_board_id = 4;
  s_test_config_1.application_crc32 = 4;
  s_test_config_1.application_size = 4;
  persist_commit(&s_test_persist_storage_1);

  s_test_config_2.controller_board_id = 5;
  s_test_config_2.application_crc32 = 5;
  s_test_config_2.application_size = 5;
  persist_commit(&s_test_persist_storage_2);

  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, config_init());
}

void test_config_get(void) {
  // This test tests to see that if config_get sets the given
  // pointer to config page 1
  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  BootloaderConfig test_input_config = { 0 };
  config_get(&test_input_config);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &test_input_config, sizeof(BootloaderConfig));
}

void test_config_commit_works(void) {
  // Testing if page 2 and page 1 are both the new page
  config_commit(&test_input_config_works);

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &test_input_config_works, sizeof(BootloaderConfig));
  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_2, &s_test_config_1, sizeof(BootloaderConfig));
}

void test_config_commit_corruption(void) {
  // Testing if page 1 is reset when the input config was corrupted
  s_function_pointer_hook = prv_change_config_page;
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, config_commit(&test_input_config_corruption));
  s_function_pointer_hook = NULL;

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(BootloaderConfig), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_2, &s_test_config_1, sizeof(BootloaderConfig));
  TEST_ASSERT_NOT_EQUAL_MEMORY(&s_test_config_2, &test_input_config_corruption,
                               sizeof(BootloaderConfig));
}

void test_config_commit_input_null(void) {
  // Testing if STATUS_CODE_INVALID_ARGS is called when input_config is null
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, config_commit(NULL));
}

StatusCode TEST_MOCK(persist_init)(PersistStorage *persist, FlashPage page, void *blob,
                                   size_t blob_size, bool overwrite) {
  if (s_function_pointer_hook != NULL) (s_function_pointer_hook)(persist, blob);

  return __real_persist_init(persist, page, blob, blob_size, overwrite);
}
