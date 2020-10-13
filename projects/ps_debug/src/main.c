#include <stdlib.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef enum PsDigi {
  PSD_VALID_1 = 0,
  PSD_VALID_2,
  PSD_VALID_3,
  PSD_DCDC_FAULT,
  NUM_PSD,
} PsDigi;

typedef enum PsAng {
  PSA_DCDC_ISENSE,
  PSA_DCDC_TEMP,
  PSA_DCDC_VSENSE,

  PSA_PWRSUP_ISENSE,
  PSA_PWRSUP_VSENSE,

  PSA_AUX_ISENSE,
  PSA_AUX_TEMP,
  PSA_AUX_VSENSE,

  NUM_PSA,
} PsAng;

static GpioAddress digi_pins[NUM_PSD] = {
  [PSD_VALID_1] = { .port = GPIO_PORT_A, .pin = 10 },
  [PSD_VALID_2] = { .port = GPIO_PORT_A, .pin = 9 },
  [PSD_VALID_3] = { .port = GPIO_PORT_A, .pin = 8 },
  [PSD_DCDC_FAULT] = { .port = GPIO_PORT_B, .pin = 0 },
};

static char *digi_names[NUM_PSD] = {
  [PSD_VALID_1] = "VALID_1",
  [PSD_VALID_2] = "VALID_2",
  [PSD_VALID_3] = "VALID_3",
  [PSD_DCDC_FAULT] = "DCDC_FAULT",
};

static GpioAddress ang_pins[NUM_PSA] = {
  [PSA_DCDC_ISENSE] = { .port = GPIO_PORT_A, .pin = 6 },
  [PSA_DCDC_TEMP] = { .port = GPIO_PORT_A, .pin = 4 },
  [PSA_DCDC_VSENSE] = { .port = GPIO_PORT_A, .pin = 1 },
  [PSA_PWRSUP_ISENSE] = { .port = GPIO_PORT_A, .pin = 7 },
  [PSA_PWRSUP_VSENSE] = { .port = GPIO_PORT_A, .pin = 2 },
  [PSA_AUX_ISENSE] = { .port = GPIO_PORT_A, .pin = 5 },
  [PSA_AUX_TEMP] = { .port = GPIO_PORT_A, .pin = 3 },
  [PSA_AUX_VSENSE] = { .port = GPIO_PORT_A, .pin = 0 },
};

static char *ang_names[NUM_PSA] = {
  [PSA_DCDC_ISENSE] = "DCDC_I",   [PSA_DCDC_TEMP] = "DCDC_T",     [PSA_DCDC_VSENSE] = "DCDC_V",
  [PSA_PWRSUP_ISENSE] = "PWRS_I", [PSA_PWRSUP_VSENSE] = "PWRS_V", [PSA_AUX_ISENSE] = "AUXB_I",
  [PSA_AUX_TEMP] = "AUXB_T",      [PSA_AUX_VSENSE] = "AUXB_V",
};

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);

  GpioSettings digi_settings = {
    GPIO_DIR_IN,      //
    GPIO_STATE_LOW,   //
    GPIO_RES_NONE,    //
    GPIO_ALTFN_NONE,  //
  };
  GpioSettings ang_settings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };
  for (PsDigi i = 0; i < NUM_PSD; i++) {
    gpio_init_pin(&digi_pins[i], &digi_settings);
  }
  for (PsAng i = 0; i < NUM_PSA; i++) {
    gpio_init_pin(&ang_pins[i], &ang_settings);
    adc_set_channel_pin(ang_pins[i], true);
  }

  while (true) {
    // collect data
    GpioState digi_states[NUM_PSD] = { 0 };
    for (PsDigi i = 0; i < NUM_PSD; i++) {
      gpio_get_state(&digi_pins[i], &digi_states[i]);
    }
    uint16_t ang_reads[NUM_PSA] = { 0 };
    for (PsAng i = 0; i < NUM_PSA; i++) {
      adc_read_converted_pin(ang_pins[i], &ang_reads[i]);
    }

    // print data nicely
    printf("=============== rand# %d\n", rand());
    printf("DIGITAL:\n");
    for (PsDigi i = 0; i < NUM_PSD; i++) {
      printf("%s - %d\n", digi_names[i], digi_states[i]);
    }
    printf("\n");
    printf("ANALOG:\n");
    for (PsAng i = 0; i < NUM_PSA; i++) {
      printf("%s - %d\n", ang_names[i], ang_reads[i]);
    }
    printf("\n");

    delay_ms(1000);
  }
}
