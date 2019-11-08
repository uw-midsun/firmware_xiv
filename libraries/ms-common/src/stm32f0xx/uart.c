// The basic idea is that we have two FIFOs, one for TX and one for TX.
// When a transmit is requested, we copy the data into the TX FIFO and use the
// TXE interrupt to clock the data out. Note that we only enable the interrupt
// when a transfer is currently in progress, as otherwise it seems to
// continuously trigger. When we receive data through the RX interrupt, we copy
// it into a buffer until we encounter a newline or the buffer is full. Once
// that occurs, we pop the data into another buffer and call the registered RX
// handler.
#include "uart.h"
#include <string.h>
#include "critical_section.h"
#include "stm32f0xx.h"

// basic idea: tx is stored in a buffer, interrupt-driven
// rx is buffered, once a newline is hit or the buffer is full, call rx_handler

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint32_t irq;
  USART_TypeDef *base;
  UartStorage *storage;
} UartPortData;

static UartPortData s_port[] = {
  [UART_PORT_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                    .periph = RCC_APB2Periph_USART1,
                    .irq = USART1_IRQn,
                    .base = USART1 },
  [UART_PORT_2] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART2,
                    .irq = USART2_IRQn,
                    .base = USART2 },
  [UART_PORT_3] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART3,
                    .irq = USART3_4_IRQn,
                    .base = USART3 },
  [UART_PORT_4] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_USART4,
                    .irq = USART3_4_IRQn,
                    .base = USART4 },
};

static void prv_tx_pop(UartPort uart);
static void prv_rx_push(UartPort uart);

static void prv_handle_irq(UartPort uart);

StatusCode uart_init(UartPort uart, UartSettings *settings, UartStorage *storage) {
  s_port[uart].rcc_cmd(s_port[uart].periph, ENABLE);

  s_port[uart].storage = storage;
  memset(storage, 0, sizeof(*storage));

  s_port[uart].storage->rx_handler = settings->rx_handler;
  s_port[uart].storage->context = settings->context;
  s_port[uart].storage->delimiter = '\n';
  fifo_init(&s_port[uart].storage->tx_fifo, s_port[uart].storage->tx_buf);
  fifo_init(&s_port[uart].storage->rx_fifo, s_port[uart].storage->rx_buf);

  GpioSettings gpio_settings = {
    .alt_function = settings->alt_fn,  //
    .resistor = GPIO_RES_PULLUP,       //
  };

  gpio_init_pin(&settings->tx, &gpio_settings);
  gpio_init_pin(&settings->rx, &gpio_settings);

  USART_InitTypeDef usart_init;
  USART_StructInit(&usart_init);
  usart_init.USART_BaudRate = settings->baudrate;
  USART_Init(s_port[uart].base, &usart_init);

  USART_ClearITPendingBit(s_port[uart].base, USART_FLAG_TXE);
  USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
  USART_ITConfig(s_port[uart].base, USART_IT_RXNE, ENABLE);

  stm32f0xx_interrupt_nvic_enable(s_port[uart].irq, INTERRUPT_PRIORITY_NORMAL);

  USART_Cmd(s_port[uart].base, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode uart_set_rx_handler(UartPort uart, UartRxHandler rx_handler, void *context) {
  bool disabled = critical_section_start();
  s_port[uart].storage->rx_handler = rx_handler;
  s_port[uart].storage->context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode uart_set_delimiter(UartPort uart, uint8_t delimiter) {
  s_port[uart].storage->delimiter = delimiter;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UartPort uart, uint8_t *tx_data, size_t len) {
  status_ok_or_return(fifo_push_arr(&s_port[uart].storage->tx_fifo, tx_data, len));

  if (USART_GetFlagStatus(s_port[uart].base, USART_FLAG_TXE) == SET) {
    prv_tx_pop(uart);
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, ENABLE);
  }

  return STATUS_CODE_OK;
}

static void prv_tx_pop(UartPort uart) {
  if (fifo_size(&s_port[uart].storage->tx_fifo) == 0) {
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
    return;
  }

  uint8_t tx_data = 0;
  fifo_pop(&s_port[uart].storage->tx_fifo, &tx_data);

  USART_SendData(s_port[uart].base, tx_data);
}

static void prv_rx_push(UartPort uart) {
  UartStorage *storage = s_port[uart].storage;

  uint8_t rx_data = USART_ReceiveData(s_port[uart].base);
  fifo_push(&storage->rx_fifo, &rx_data);

  size_t num_bytes = fifo_size(&storage->rx_fifo);
  if (rx_data == storage->delimiter || num_bytes == UART_MAX_BUFFER_LEN) {
    storage->rx_line_buf[num_bytes] = '\0';
    fifo_pop_arr(&storage->rx_fifo, storage->rx_line_buf, num_bytes);

    if (storage->rx_handler != NULL) {
      storage->rx_handler(storage->rx_line_buf, num_bytes, storage->context);
    }
  }
}

static void prv_handle_irq(UartPort uart) {
  if (USART_GetITStatus(s_port[uart].base, USART_IT_TXE) == SET) {
    prv_tx_pop(uart);
    USART_ClearITPendingBit(s_port[uart].base, USART_IT_TXE);
  }

  if (USART_GetITStatus(s_port[uart].base, USART_IT_RXNE) == SET) {
    prv_rx_push(uart);
  }

  // Clear overrun flag
  USART_ClearITPendingBit(s_port[uart].base, USART_IT_ORE);
}

void USART1_IRQHandler(void) {
  prv_handle_irq(UART_PORT_1);
}

void USART2_IRQHandler(void) {
  prv_handle_irq(UART_PORT_2);
}

void USART3_4_IRQHandler(void) {
  prv_handle_irq(UART_PORT_3);
  prv_handle_irq(UART_PORT_4);
}
