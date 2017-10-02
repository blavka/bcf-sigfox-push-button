#include <application.h>

bc_led_t led;
bc_button_t button;
bc_module_sigfox_t sigfox_module;
uint16_t event_count = 0;

typedef enum
{
    SIGFOX_HEADER_RESET = 0,
    SIGFOX_HEADER_BUTTON = 1

} sigfox_header_t;

void transmit_reset_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_relative(50);

        return;
    }

    bc_scheduler_unregister(bc_scheduler_get_current_task_id());

    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint8_t buffer[1] = { SIGFOX_HEADER_RESET };

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void transmit_button_task(void *param)
{
    (void) param;

    if (!bc_module_sigfox_is_ready(&sigfox_module))
    {
        bc_scheduler_plan_current_relative(50);

        return;
    }

    bc_scheduler_unregister(bc_scheduler_get_current_task_id());

    bc_led_set_mode(&led, BC_LED_MODE_ON);

    uint8_t buffer[3];
    buffer[0] = SIGFOX_HEADER_BUTTON;
    buffer[1] = event_count >> 8;
    buffer[2] = event_count;

    bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
}

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MODULE_SIGFOX_EVENT_ERROR)
    {
        bc_led_pulse(&led, 0);

        bc_led_set_mode(&led, BC_LED_MODE_BLINK);
    }
    else if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        event_count++;

        bc_scheduler_register(transmit_button_task, NULL, 0);
    }
}

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    bc_module_sigfox_init(&sigfox_module, BC_MODULE_SIGFOX_REVISION_R2);
    bc_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_scheduler_register(transmit_reset_task, NULL, 0);

    bc_led_set_mode(&led, BC_LED_MODE_OFF);
}
