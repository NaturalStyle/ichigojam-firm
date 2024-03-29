#ifndef __I2C_H__
#define __I2C_H__

#define I2C_DEFAULT_BPS 400000
#define TIMEOUT_US 1000000

static int i2c_baudrate = I2C_DEFAULT_BPS;

/* Initialize I2C module  0:ok 1:_g.err */
int i2c0_init() {
    i2c_init(i2c_default, I2C_DEFAULT_BPS);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    return 0;
}

// -1:ad error 1:ok 0:i2c error
INLINE int IJB_i2c(uint8 writemode, uint16* param) {
    return 0;
}

#endif