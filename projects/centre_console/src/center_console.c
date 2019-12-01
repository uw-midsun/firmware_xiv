#include <stdbool.h>
#include "center_console.h"

static CenterConsoleStorage s_cc_storage = { 0 };

static void prv_center_console_btn_callback(const GpioAddress *address, void *context) {
    GpioItCallbackContext *callback_context = (GpioItCallbackContext *) context;
    CenterConsoleInputLink* input_link = callback_context->input_link;
    Event *e = callback_context->e;

    // toggle the led on the button
    gpio_toggle_state(&input_link->btn_addr);

    // process can event
    while (event_process(e) != STATUS_CODE_OK) {
    }
    can_process_event(e);
}

static StatusCode prv_init_input_btn(CenterConsoleInputLink* input_link) {
    const GpioSettings button_input_settings = {
        .direction = GPIO_DIR_IN,
        .state = GPIO_STATE_LOW,
        .resistor = GPIO_RES_NONE,
        .alt_function = GPIO_ALTFN_NONE,
    };

    const InterruptSettings interrupt_settings = {
        .type = INTERRUPT_TYPE_INTERRUPT,
        .priority = INTERRUPT_PRIORITY_NORMAL,
    };

    status_ok_or_return(gpio_init_pin(&input_link->btn_addr, &button_input_settings));

    GpioItCallback callback = &prv_center_console_btn_callback;

    Event e = { 0 };
    GpioItCallbackContext callback_context = {
        .input_link = input_link,
        .e = &e,
    };

    status_ok_or_return(gpio_it_register_interrupt(&input_link->btn_addr, &interrupt_settings,
                                    INTERRUPT_EDGE_RISING_FALLING, callback, &callback_context));

    return STATUS_CODE_OK;
}


StatusCode initialize_center_console(CenterConsoleStorage* cc_storage) {
    // init all the buttons here
    status_ok_or_return(prv_init_input_btn(&cc_storage->power_input));
    status_ok_or_return(prv_init_input_btn(&cc_storage->drive_input));
    status_ok_or_return(prv_init_input_btn(&cc_storage->neutral_input));
    status_ok_or_return(prv_init_input_btn(&cc_storage->reverse_input));
    status_ok_or_return(prv_init_input_btn(&cc_storage->hazards_input));
    status_ok_or_return(prv_init_input_btn(&cc_storage->low_beam_input));

    return STATUS_CODE_OK;
}
