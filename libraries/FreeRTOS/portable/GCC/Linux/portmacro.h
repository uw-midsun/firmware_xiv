#pragma once

// This provides port-specific macros for the Linux FreeRTOS port
#include <stdint.h>

typedef long BaseType_t;
typedef unsigned long UBaseType_t;
typedef unsigned long StackType_t;

#if (configUSE_16_BIT_TICKS == 1)
  typedef uint16_t TickType_t;
	#define portMAX_DELAY ((uint16_t)0xffff)
#else
  typedef uint32_t TickType_t;
	#define portMAX_DELAY ((uint32_t)0xffffffff)
#endif

// TODO: Check this alignment setting?
#define portBYTE_ALIGNMENT 8

// Critical sections
#define portENTER_CRITICAL()
#define portEXIT_CRITICAL()

// Interrupts
#define portDISABLE_INTERRUPTS()
#define portENABLE_INTERRUPTS()
#define portYIELD()

#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) void vFunction(void *pvParameters)
#define portTASK_FUNCTION(vFunction, pvParameters) void vFunction(void *pvParameters)

// Architecture specific defines

// On x86, the stack grows downwards (ie. from higher addresses to lower)
#define portSTACK_GROWTH			(-1)
