#include "output_config.h"

#include "pin_defs.h"

const OutputConfig COMBINED_OUTPUT_CONFIG = {
  .specs = {
    [FRONT_OUTPUT_SPEAKER] = {
      .type = OUTPUT_TYPE_BTS7040,
      .spec = &(OutputBts7040Spec){
        .enable_pin = FRONT_PIN_SPEAKER_EN,
        .mux_selection = FRONT_MUX_SEL_SPEAKER,
      },
    },
  },
};
