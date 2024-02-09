#define LED 20
#define BTN 28
#define IN1 27
#define IN2 26
#define IN3 22
#define IN4 21
#define OUT1 6
#define OUT2 7
#define OUT3 8
#define OUT4 9
#define OUT5 10
#define OUT6 11

uint8 in_pins[] = { IN1, IN2, IN3, IN4, OUT1, OUT2, OUT3, OUT4, BTN, OUT5, OUT6 };
uint8 out_pins[] = { OUT1, OUT2, OUT3, OUT4, OUT5, OUT6, LED, IN1, IN2, IN3, IN4 };

void io_init() {
    for (int i = 0; i < 4; i++) {
        uint8 pin = in_pins[i];
        gpio_init(pin);
        gpio_pull_up(pin);
    }
    for (int i = 0; i < 6; i++) {
        uint8 pin = out_pins[i];
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_init(BTN);
    gpio_set_dir(BTN, GPIO_OUT);
}

//TODO 反応するキーを絞るか検討する
int IJB_btn(int n) {
    if (n == -1) {
        int res = 0;
        for (uint8_t i = 0; i < 6; i++) {
            uint8_t keycode = now_key_report.keycode[i];
            uint8_t ascii = keycode2ascii[keycode][0];
            switch (ascii) {
            case LEFT:
                res |= 1 << 0;
                break;
            case RIGHT:
                res |= 1 << 1;
                break;
            case UP:
                res |= 1 << 2;
                break;
            case DOWN:
                res |= 1 << 3;
                break;
            case ' ':
                res |= 1 << 4;
                break;
            case 'x'://keycode2ascii[][0]は小文字を取得する
                res |= 1 << 5;
                break;
            }
        }
        return res;
    } else {
        for (uint8_t i = 0; i < 6; i++) {
            uint8_t keycode = now_key_report.keycode[i];
            if (keycode) {
                if (n == keycode2ascii[keycode][0] || n == keycode2ascii[keycode][1]) {
                    return true;
                }
            }
        }
        return false;
    }
}

int IJB_in() {
    int res = 0;
    for (int i = 0; i < 11; i++) {
        bool bit = gpio_get(in_pins[i]);
        res |= bit << i;
        printf("%d\n", bit);
    }
    return res;
}

void IJB_out(int port, int st) {
    if (port == 0) {
        for (int i = 0; i < 11; i++) {
            gpio_put(out_pins[i], st & (1 << i));
        }
    } else {
        uint8 pin = out_pins[port - 1];
        if (st >= 0) {
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, st);
        } else if (st == -1) {
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
        } else if (st == -2) {
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
        }
    }
}

INLINE void IJB_led(int st) {
    IJB_out(7, st != 0);
}

INLINE void IJB_clo() {
    io_init();
}