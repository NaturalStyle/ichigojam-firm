#include "pico/stdlib.h"
#include <stdio.h>

int main() {
    int led = 21;
    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);
    while (true) {
        gpio_put(led, 1);
        sleep_ms(1000);
        gpio_put(led, 0);
        sleep_ms(1000);
    }
}