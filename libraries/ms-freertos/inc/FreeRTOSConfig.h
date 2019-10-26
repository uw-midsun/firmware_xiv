#pragma once

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
#include <stdint.h>
extern uint32_t SystemCoreClock;
#endif

#define configUSE_PREEMPTION 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE 0
#define configCPU_CLOCK_HZ (SystemCoreClock)
#define configTICK_RATE_HZ ((TickType_t)1000)
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE ((uint16_t)128)
#define configMAX_TASK_NAME_LEN 16
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 0
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configQUEUE_REGISTRY_SIZE 8
#define configUSE_QUEUE_SETS 0
#define configUSE_TIME_SLICING 1
#define configUSE_NEWLIB_REENTRANT 0
#define configENABLE_BACKWARD_COMPATIBILITY 0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

// Memory allocation related definitions
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 0
#define configTOTAL_HEAP_SIZE 0
#define configAPPLICATION_ALLOCATED_HEAP 0

// Hook function related definitions
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_MALLOC_FAILED_HOOK 0
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0

// Run time and task stats gathering related definitions
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

// Co-routine definitions
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES 2

// Software timer related functions
#define configUSE_TIMERS 0
#define configTIMER_TASK_PRIORITY 3
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

// Optional functions - most linkers will remove unused functions anyway
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xResumeFromISR 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 0
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_uxTaskGetStackHighWaterMark 0
#define INCLUDE_xTaskGetIdleTaskHandle 0
#define INCLUDE_eTaskGetState 0
#define INCLUDE_xEventGroupSetBitFromISR 1
#define INCLUDE_xTimerPendFunctionCall 0
#define INCLUDE_xTaskAbortDelay 0
#define INCLUDE_xTaskGetHandle 0
#define INCLUDE_xTaskResumeFromISR 1

// Interrupt nesting behaviour configuration

// Cortex-M specific definitions.
#ifdef __NVIC_PRIO_BITS
// __BVIC_PRIO_BITS will be specified when CMSIS is being used.
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 2
#endif

// The lowest interrupt priority that can be used in a call to a "set priority"
// function
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 3

// The highest interrupt priority that can be used by any interrupt service
// routine that makes calls to interrupt safe FreeRTOS API functions.
//
// DO NOT CALL INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT
// HAS A HIGHER PRIORITY THAN THIS! (higher priorities are lower numeric
// values.)
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 3

// Interrupt priorities used by the kernel port layer itself. These are generic
// to all Cortex-M ports, and do not rely on any particular library functions.
#define configKERNEL_INTERRUPT_PRIORITY \
  (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
// !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
// See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

// Normal assert() semantics without relying on the provision of an assert.h
// header file
#define configASSERT(x)       \
  if ((x) == 0) {             \
    taskDISABLE_INTERRUPTS(); \
    for (;;)                  \
      ;                       \
  }

// Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
// standard names
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler

// IMPORTANT: This define MUST be commented when used with STM32Cube firmware,
// to prevent overwriting SysTick_Handler defined within STM32Cube HAL
#define xPortSysTickHandler SysTick_Handler

// Section where parameter definitions can be added (for instance, to override
// default ones in FreeRTOS.h)
