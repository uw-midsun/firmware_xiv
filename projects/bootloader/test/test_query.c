#include "config.h"
#include "query.h"

BootloaderConfig config = {
  .controller_board_id = 1,
  .controller_board_name = "name",
  .git_version = "git",
  .project_name = "proj name",
  .project_info = "",
  .project_present = false,
};

void setup_test(void) {}

void teardown_test(void) {}

void test_init(void) {
  query_init(&config);
}
