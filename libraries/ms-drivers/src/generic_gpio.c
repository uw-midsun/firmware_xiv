#include "generic_gpio.h"

#include "log.h"
#include "status.h"

StatusCode generic_gpio_init_gpio_pin(const GpioAddress *address, const GpioSettings *settings,
                                      GenGpioAddress *gen_addr) {
  status_ok_or_return(gpio_init_pin(address, settings));
  gen_addr->type = GEN_GPIO_TYPE_GPIO;
  gen_addr->gpio_addr = address;
  return STATUS_CODE_OK;
}

StatusCode generic_gpio_init_pca9539r(const Pca9539rGpioAddress *address,
                                      const Pca9539rGpioSettings *settings,
                                      GenGpioAddress *gen_addr) {
  status_ok_or_return(pca9539r_gpio_init_pin(address, settings));
  gen_addr->type = GEN_GPIO_TYPE_PCA9539R;
  gen_addr->pca_addr = address;
  return STATUS_CODE_OK;
}

StatusCode generic_gpio_init_mcp23008(const Mcp23008GpioAddress *address,
                                      const Mcp23008GpioSettings *settings,
                                      GenGpioAddress *gen_addr) {
  status_ok_or_return(mcp23008_gpio_init_pin(address, settings));
  gen_addr->type = GEN_GPIO_TYPE_MCP23008;
  gen_addr->mcp_addr = address;
  return STATUS_CODE_OK;
}

StatusCode generic_gpio_set_state(const GenGpioAddress *address, GenGpioState state) {
  switch (address->type) {
    case GEN_GPIO_TYPE_GPIO:
      return gpio_set_state(address->gpio_addr, state);
    case GEN_GPIO_TYPE_PCA9539R:
      return pca9539r_gpio_set_state(address->pca_addr, state);
    case GEN_GPIO_TYPE_MCP23008:
      return mcp23008_gpio_set_state(address->mcp_addr, state);
    default:
      return STATUS_CODE_UNREACHABLE;
  }
}

StatusCode generic_gpio_get_state(const GenGpioAddress *address, GenGpioState *state) {
  switch (address->type) {
    case GEN_GPIO_TYPE_GPIO:
      return gpio_get_state(address->gpio_addr, (GpioState *)state);
    case GEN_GPIO_TYPE_PCA9539R:
      return pca9539r_gpio_get_state(address->pca_addr, (Pca9539rGpioState *)state);
    case GEN_GPIO_TYPE_MCP23008:
      return mcp23008_gpio_get_state(address->mcp_addr, (Mcp23008GpioState *)state);
    default:
      return STATUS_CODE_UNREACHABLE;
  }
}
