#include "FreeRTOS.h"
// This is needed to statically allocate the memory for the idle task. We have
// configSUPPORT_STATIC_ALLOCATION set, and so the application must provide an
// implementation of vApplicationGetIdleTaskMemory.

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  // Pass out a pointer to the StaticTask_t structure in which the Idle task's
  // state will be stored.
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  // Pass out the array that will be used as the Idle task's stack.
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
