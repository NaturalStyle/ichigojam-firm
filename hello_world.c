// cmake -DPICO_SDK_PATH=/Users/morita/dev/raspberry_pi/pico-sdk
// -DPICO_TOOLCHAIN_PATH=/opt/homebrew ..
// sudo screen /dev/tty.usbmodem11101 115200
#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    for(int i=0;i<60;i++){
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
    printf("END");
    return 0;
}
