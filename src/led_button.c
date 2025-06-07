#include "led_button.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#define NUM_PAIRS 4

// Define the LED and button aliases one by one
static const struct gpio_dt_spec leds[NUM_PAIRS] = {
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios, {0}),
};

static const struct gpio_dt_spec buttons[NUM_PAIRS] = {
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw2), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw3), gpios, {0}),
};

static struct gpio_callback button_cbs[NUM_PAIRS];
static bool led_states[NUM_PAIRS] = {false, false, false, false};

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // Identify which button was pressed by comparing cb pointer
    for (int i = 0; i < NUM_PAIRS; i++) {
        if (cb == &button_cbs[i]) {
            led_states[i] = !led_states[i];
            gpio_pin_set(leds[i].port, leds[i].pin, (int)led_states[i]);
            printk("Button %d pressed, LED %d %s\n", i+1, i+1, led_states[i] ? "ON" : "OFF");
            break;
        }
    }
}

int button_led_init(void)
{
    int ret;

    for (int i = 0; i < NUM_PAIRS; i++) {
        if (!device_is_ready(leds[i].port)) {
            printk("LED%d device not ready\n", i+1);
            return -ENODEV;
        }
        if (!device_is_ready(buttons[i].port)) {
            printk("Button%d device not ready\n", i+1);
            return -ENODEV;
        }

        ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
        if (ret) {
            printk("Failed to configure LED%d\n", i+1);
            return ret;
        }

        ret = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
        if (ret) {
            printk("Failed to configure Button%d\n", i+1);
            return ret;
        }

        ret = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_ACTIVE);
        if (ret) {
            printk("Failed to configure interrupt for Button%d\n", i+1);
            return ret;
        }

        gpio_init_callback(&button_cbs[i], button_pressed, BIT(buttons[i].pin));
        gpio_add_callback(buttons[i].port, &button_cbs[i]);
    }

    printk("button_led module initialized for %d button-LED pairs\n", NUM_PAIRS);
    return 0;
}
