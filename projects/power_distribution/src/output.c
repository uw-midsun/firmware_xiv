#include "output.h"

#include <stdint.h>

#include "status.h"

static OutputType s_output_types[NUM_OUTPUTS] = {
  [FRONT_OUTPUT_CENTRE_CONSOLE] = OUTPUT_TYPE_BTS7200,
};

StatusCode output_init(OutputConfig *config) {
  return STATUS_CODE_OK;
}

StatusCode output_set_state(Output output, OutputState state) {
  return STATUS_CODE_OK;
}

StatusCode output_read_current(Output output, uint16_t *current) {
  return STATUS_CODE_OK;
}
