#include "stm32f0xx_interrupt.h"

#include <stdbool.h>
#include <stdint.h>

#include "interrupt_def.h"
#include "status.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"

// From the User Manual.
// Channels are NVIC channels which handle prioritization of interupts.
#define NUM_STM32F0XX_INTERRUPT_CHANNELS 32
// Lines are external interrupt channels often grouped to a single interrupt.
#define NUM_STM32F0XX_INTERRUPT_LINES 32

static InterruptPriority s_stm32f0xx_interrupt_priorities[NUM_STM32F0XX_INTERRUPT_CHANNELS];

void stm32f0xx_interrupt_init(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  for (int i = 0; i < NUM_STM32F0XX_INTERRUPT_CHANNELS; i++) {
    s_stm32f0xx_interrupt_priorities[i] = NUM_INTERRUPT_PRIORITIES;
  }
}

StatusCode stm32f0xx_interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  if (priority >= NUM_INTERRUPT_PRIORITIES || irq_channel >= NUM_STM32F0XX_INTERRUPT_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_stm32f0xx_interrupt_priorities[irq_channel] == priority) {
    return STATUS_CODE_OK;
  } else if (s_stm32f0xx_interrupt_priorities[irq_channel] != NUM_INTERRUPT_PRIORITIES) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Priority already set.");
  }

  s_stm32f0xx_interrupt_priorities[irq_channel] = priority;
  NVIC_InitTypeDef init_struct = {
    .NVIC_IRQChannel = irq_channel,
    .NVIC_IRQChannelPriority = priority,
    .NVIC_IRQChannelCmd = ENABLE,
  };

  NVIC_Init(&init_struct);

  return STATUS_CODE_OK;
}

StatusCode stm32f0xx_interrupt_exti_enable(uint8_t line, const InterruptSettings *settings,
                                           InterruptEdge edge) {
  if (line >= NUM_STM32F0XX_INTERRUPT_LINES || settings->type >= NUM_INTERRUPT_TYPES ||
      edge >= NUM_INTERRUPT_EDGES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_InitTypeDef init_struct = {
    .EXTI_Line = (uint32_t)0x01 << line,
    .EXTI_Mode = 0x04 * settings->type,
    .EXTI_Trigger = 0x08 + 0x04 * edge,
    .EXTI_LineCmd = ENABLE,
  };
  EXTI_Init(&init_struct);

  return STATUS_CODE_OK;
}

StatusCode stm32f0xx_interrupt_exti_trigger(uint8_t line) {
  // It is not possible to manually trigger interrupts for lines 18 and > 22.
  if (line == 18 || line > 22) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_GenerateSWInterrupt((uint32_t)0x01 << line);
  return STATUS_CODE_OK;
}

StatusCode stm32f0xx_interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  if (line >= NUM_STM32F0XX_INTERRUPT_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *pending_bit = (uint8_t)EXTI_GetITStatus((uint32_t)0x01 << line);
  return STATUS_CODE_OK;
}

StatusCode stm32f0xx_interrupt_exti_clear_pending(uint8_t line) {
  if (line >= NUM_STM32F0XX_INTERRUPT_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_ClearITPendingBit((uint32_t)0x01 << line);
  return STATUS_CODE_OK;
}

StatusCode stm32f0xx_interrupt_exti_mask_set(uint8_t line, bool masked) {
  if (line >= NUM_STM32F0XX_INTERRUPT_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  
  if (masked) {
    EXTI->IMR &= ~((uint32_t)0x01 << line);
  } else {
    EXTI->IMR |= (uint32_t)0x01 << line;
  }

  return STATUS_CODE_OK;
}
