// Generic GPIO interface used when interfacing with multiple GPIO expanders and/or the onboard GPIO pins.
// Requires the GPIO type to be used to be initialized, as well as any other type-specific requirements like I2C.
#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "mcp23008_gpio_expander.h"

// Specific GPIO types to be accessed through generic GPIO.
typedef enum {
    GEN_GPIO_TYPE_GPIO = 0,
    GEN_GPIO_TYPE_PCA9539R,
    GEN_GPIO_TYPE_MCP23008,
    NUM_GEN_GPIO_TYPES,
} GenGpioType;

// Possible generic GPIO states.
typedef enum {
    GEN_GPIO_STATE_LOW = 0,
    GEN_GPIO_STATE_HIGH,
    NUM_GEN_GPIO_STATES,
} GenGpioState;

// Generic GPIO address. Holds a pointer to the correct type and stores the correct type 
// to be used later.
typedef struct {
    GenGpioType type;
    GpioAddress *gpio_addr;
    Pca9539rGpioAddress *pca_addr;
    Mcp23008GpioAddress *mcp_addr;
} GenGpioAddress;

// Initialize the given onboard pin and configure the generic address with  it.
StatusCode generic_gpio_init_gpio_pin(const GpioAddress *address, const GpioSettings *settings, GenGpioAddress *gen_addr);

// Initialize the given PCA9539R pin and configure the generic address with it.
StatusCode generic_gpio_init_pca9539r(const Pca9539rGpioAddress *address, const Pca9539rGpioSettings *settings, GenGpioAddress *gen_addr);

// Initialize the given MCP23008 pin and configure the generic address with it.
StatusCode generic_gpio_init_mcp23008(const Mcp23008GpioAddress *address, const Mcp23008GpioSettings *settings, GenGpioAddress *gen_addr);

// WARNING: the below functions currently cast between different state types, which assumes that LOW = 0 and HIGH = 1.
// Routes to the set function for address.
StatusCode generic_gpio_set_state(const GenGpioAddress *address, GenGpioState state);

// Routes to the get function for address.
StatusCode generic_gpio_get_state(const GenGpioAddress *address, GenGpioState *state);