#include "pico/stdlib.h"
#include <stdio.h>

int main() {
    int led = 16;
    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);
    int button_in = 14;
    gpio_init(button_in);
    gpio_set_dir(button_in, GPIO_IN);
    gpio_pull_up(button_in);
    while (true) {
        gpio_put(led, gpio_get(button_in));
    }
}