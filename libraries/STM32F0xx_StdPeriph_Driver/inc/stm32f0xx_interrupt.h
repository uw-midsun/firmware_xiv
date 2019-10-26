#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "interrupt_def.h"
#include "status.h"

// Initializes interrupt internals. If called multiple times the subsequent attempts will clear
// everything resulting in needing to re initialize al interrupts.
void stm32f0xx_interrupt_init(void);

// Enables the nested interrupt vector controller for a given channel.
StatusCode stm32f0xx_interrupt_nvic_enable(uint8_t irq_channel, InterruptPriority priority);

// Enables the external interrupt line with the given settings.
StatusCode stm32f0xx_interrupt_exti_enable(uint8_t line, const InterruptSettings *settings,
                                           InterruptEdge edge);

// Triggers a software interrupt on a given external interrupt.
StatusCode stm32f0xx_interrupt_exti_trigger(uint8_t line);

// Get the pending flag for an external interrupt.
StatusCode stm32f0xx_interrupt_exti_get_pending(uint8_t line, uint8_t *pending_bit);

// Clears the pending flag for an external interrupt.
StatusCode stm32f0xx_interrupt_exti_clear_pending(uint8_t line);

// Masks or clears the external interrupt on the given line.
StatusCode stm32f0xx_interrupt_exti_mask_set(uint8_t line, bool masked);
