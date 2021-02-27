#include "bootloader_mcu.h"
#include "config.h"
#include "crc32.h"
#include "flash.h"
#include "log.h"
#include "persist.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

extern StatusCode __real_persist_init(PersistStorage *persist, FlashPage page, void *blob,
                                      size_t blob_size, bool overwrite);

static PersistStorage s_test_persist_storage_1 = { 0 };
static BootloaderConfig s_test_config_1 = { 0 };

static PersistStorage s_test_persist_storage_2 = { 0 };
static BootloaderConfig s_test_config_2 = { 0 };

#define BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_1_START))
#define BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE (FLASH_ADDR_TO_PAGE(BOOTLOADER_CONFIG_PAGE_2_START))

static void (*s_function_pointer_hook)() = NULL;

static void prv_change_config_page(void) {
  // This function chnages the values in config page 1
  // This is to mimic the event in which page 1 gets corrupted
  // during config_commit
  s_test_config_1.controller_board_ID = 6;
  s_test_config_1.app_code = 6;
  s_test_config_1.app_size = 6;
  persist_commit(&s_test_persist_storage_1);
}

void setup_test(void) {
  flash_init();
  soft_timer_init();
  crc32_init();

  flash_erase(BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE);
  flash_erase(BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE);

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(s_test_config_1), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(s_test_config_2), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  // Input "test values" for the two config pages
  s_test_config_1.CRC32 = 0;
  s_test_config_2.CRC32 = 0;

  s_test_config_1.CRC32 = crc32_arr((uint8_t *)&s_test_config_1, sizeof(s_test_config_1));
  s_test_config_2.CRC32 = crc32_arr((uint8_t *)&s_test_config_2, sizeof(s_test_config_2));
  persist_commit(&s_test_persist_storage_1);
  persist_commit(&s_test_persist_storage_2);

  TEST_ASSERT_OK(config_init());
}
void teardown_test(void) {}

void test_config_init(void) {
  // Test 1, testing if config_init works under normal circumstances
  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(s_test_config_1), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(s_test_config_2), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &s_test_config_2, sizeof(s_test_config_1));

  // Test 2, testing if config_init works for a faulty page (in this case, page 2)
  // Below, I'm purposefully changing config 2 so that it becomes faulty
  s_test_config_2.controller_board_ID = 5;
  s_test_config_2.app_code = 5;
  s_test_config_2.app_size = 5;
  persist_commit(&s_test_persist_storage_2);

  TEST_ASSERT_OK(config_init());

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(s_test_config_1), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(s_test_config_2), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &s_test_config_2, sizeof(s_test_config_1));

  // Test 3, testing if config_init throws the correct error if both pages are corrupted
  s_test_config_1.controller_board_ID = 6;
  s_test_config_1.app_code = 6;
  s_test_config_1.app_size = 6;
  persist_commit(&s_test_persist_storage_1);

  s_test_config_2.controller_board_ID = 5;
  s_test_config_2.app_code = 5;
  s_test_config_2.app_size = 5;
  persist_commit(&s_test_persist_storage_2);

  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, config_init());
}

void test_config_get(void) {
  // This test tests to see that if config_get sets the given
  // pointer to config page 1
  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(s_test_config_1), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  BootloaderConfig test_input_config = { 0 };
  TEST_ASSERT_OK(config_get(&test_input_config));

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &test_input_config, sizeof(s_test_config_1));
}

void test_config_commit(void) {
  // I need to create an input config to give config_commit();
  // I use random values for the input_config
  BootloaderConfig test_input_config = { .CRC32 = 1,
                                         .controller_board_ID = 1,
                                         .controller_board_name = { 'a', '\0' },
                                         .project_present = true,
                                         .project_name = { 'a', '\0' },
                                         .project_info = { 'a', '\0' },
                                         .git_version = { 'a', '\0' },
                                         .app_code = 1,
                                         .app_size = 1 };

  // Test 1, testing if page 2 and page 1 are both the new page
  config_commit(&test_input_config);

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(s_test_config_1), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(s_test_config_2), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_1, &test_input_config, sizeof(s_test_config_1));
  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_2, &s_test_config_1, sizeof(s_test_config_2));

  // Test 2, testing if page 1 is reset when the input config was corrupted
  s_function_pointer_hook = &prv_change_config_page;
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, config_commit(&test_input_config));
  s_function_pointer_hook = NULL;

  persist_init(&s_test_persist_storage_1, BOOTLOADER_CONFIG_PAGE_1_FLASH_PAGE, &s_test_config_1,
               sizeof(s_test_config_1), false);
  persist_ctrl_periodic(&s_test_persist_storage_1, false);

  persist_init(&s_test_persist_storage_2, BOOTLOADER_CONFIG_PAGE_2_FLASH_PAGE, &s_test_config_2,
               sizeof(s_test_config_2), false);
  persist_ctrl_periodic(&s_test_persist_storage_2, false);

  TEST_ASSERT_EQUAL_MEMORY(&s_test_config_2, &s_test_config_1, sizeof(s_test_config_2));
}

StatusCode TEST_MOCK(persist_init)(PersistStorage *persist, FlashPage page, void *blob,
                                   size_t blob_size, bool overwrite) {
  if (s_function_pointer_hook != NULL) (*s_function_pointer_hook)();
  __real_persist_init(persist, page, blob, blob_size, overwrite);

  return STATUS_CODE_OK;
}
