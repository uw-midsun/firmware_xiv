#include "uart.h"

StatusCode uart_init(UartPort uart, UartSettings *settings, UartStorage *storage) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_set_rx_handler(UartPort uart, UartRxHandler rx_handler, void *context) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_set_delimiter(UartPort uart, uint8_t delimiter) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode uart_tx(UartPort uart, uint8_t *tx_data, size_t len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
