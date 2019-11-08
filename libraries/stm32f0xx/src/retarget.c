// Retargets STDOUT to UART
#include "retarget.h"
#include <stdio.h>
#include "retarget_cfg.h"
#include "stm32f0xx.h"

static void prv_init_gpio(void) {
  RETARGET_CFG_UART_GPIO_ENABLE_CLK();

  GPIO_PinAFConfig(RETARGET_CFG_UART_GPIO_PORT, RETARGET_CFG_UART_GPIO_TX,
                   RETARGET_CFG_UART_GPIO_ALTFN);
  GPIO_PinAFConfig(RETARGET_CFG_UART_GPIO_PORT, RETARGET_CFG_UART_GPIO_RX,
                   RETARGET_CFG_UART_GPIO_ALTFN);

  GPIO_InitTypeDef gpio_init = {
    .GPIO_Pin = (1 << RETARGET_CFG_UART_GPIO_TX) | (1 << RETARGET_CFG_UART_GPIO_RX),
    .GPIO_Mode = GPIO_Mode_AF,
    .GPIO_Speed = GPIO_Speed_10MHz,
    .GPIO_OType = GPIO_OType_PP,
    .GPIO_PuPd = GPIO_PuPd_UP,
  };

  GPIO_Init(RETARGET_CFG_UART_GPIO_PORT, &gpio_init);
}

void retarget_init(void) {
  RETARGET_CFG_UART_ENABLE_CLK();
  prv_init_gpio();

  USART_InitTypeDef usart_init;
  USART_StructInit(&usart_init);
  usart_init.USART_BaudRate = 115200;
  USART_Init(RETARGET_CFG_UART, &usart_init);

  USART_Cmd(RETARGET_CFG_UART, ENABLE);
}

int _write(int fd, char *ptr, int len) {
  uint32_t primask = __get_PRIMASK();
  __disable_irq();

  for (int i = 0; i < len; i++) {
    while (USART_GetFlagStatus(RETARGET_CFG_UART, USART_FLAG_TXE) == RESET) {
    }
    USART_SendData(RETARGET_CFG_UART, (uint8_t) * (ptr + i));
  }

  if (!primask) {
    __enable_irq();
  }

  return len;
}

__attribute__((naked, section(".hardfault"))) void HardFault_Handler(void) {
  // Get the appropriate stack pointer, depending on our mode,
  // and use it as the parameter to the C handler. This function
  // will never return

  __asm(
      ".syntax unified\n"
      "MOVS   R0, #4  \n"
      "MOV    R1, LR  \n"
      "TST    R0, R1  \n"
      "BEQ    _MSP    \n"
      "MRS    R0, PSP \n"
      "B      HardFault_HandlerC      \n"
      "_MSP:  \n"
      "MRS    R0, MSP \n"
      "B      HardFault_HandlerC      \n"
      ".syntax divided\n");
}

__attribute__((used, section(".hardfault"))) void HardFault_HandlerC(uint32_t *hardfault_args) {
  volatile uint32_t stacked_r0;
  volatile uint32_t stacked_r1;
  volatile uint32_t stacked_r2;
  volatile uint32_t stacked_r3;
  volatile uint32_t stacked_r12;
  volatile uint32_t stacked_lr;
  volatile uint32_t stacked_pc;
  volatile uint32_t stacked_psr;
  volatile uint32_t _CFSR;
  volatile uint32_t _HFSR;
  volatile uint32_t _DFSR;
  volatile uint32_t _AFSR;
  volatile uint32_t _BFAR;
  volatile uint32_t _MMAR;

  stacked_r0 = ((uint32_t)hardfault_args[0]);
  stacked_r1 = ((uint32_t)hardfault_args[1]);
  stacked_r2 = ((uint32_t)hardfault_args[2]);
  stacked_r3 = ((uint32_t)hardfault_args[3]);
  stacked_r12 = ((uint32_t)hardfault_args[4]);
  stacked_lr = ((uint32_t)hardfault_args[5]);
  stacked_pc = ((uint32_t)hardfault_args[6]);
  stacked_psr = ((uint32_t)hardfault_args[7]);

  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  _CFSR = (*((volatile uint32_t *)(0xE000ED28)));
  // Hard Fault Status Register
  _HFSR = (*((volatile uint32_t *)(0xE000ED2C)));

  // Debug Fault Status Register
  _DFSR = (*((volatile uint32_t *)(0xE000ED30)));

  // Auxiliary Fault Status Register
  _AFSR = (*((volatile uint32_t *)(0xE000ED3C)));

  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  _MMAR = (*((volatile uint32_t *)(0xE000ED34)));
  // Bus Fault Address Register
  _BFAR = (*((volatile uint32_t *)(0xE000ED38)));

  printf("R0: 0x%lx\n", stacked_r0);
  printf("R1: 0x%lx\n", stacked_r1);
  printf("R2: 0x%lx\n", stacked_r2);
  printf("R3: 0x%lx\n", stacked_r3);

  printf("R12: 0x%lx\n", stacked_r12);
  printf("LR: 0x%lx\n", stacked_lr);
  printf("PC: 0x%lx\n", stacked_pc);
  printf("PSR: 0x%lx\n", stacked_psr);

  printf("CFSR: 0x%lx\n", _CFSR);
  printf("HFSR: 0x%lx\n", _HFSR);
  printf("DFSR: 0x%lx\n", _DFSR);
  printf("AFSR: 0x%lx\n", _AFSR);
  printf("MMAR: 0x%lx\n", _MMAR);
  printf("BFAR: 0x%lx\n", _BFAR);

  __asm("BKPT #0\n");  // Break into the debugger
}
