#define LED 2
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

#define IO_PIN_NUM 11

//ラズパイの動作クロックは252MHz(PicoDVIでオーバークロックしている)、IchigoJamのPWMは1周期20msなので50Hz
//https://rikei-tawamure.com/entry/2021/02/08/213335#PWM%E7%94%A8%E3%82%AB%E3%82%A6%E3%83%B3%E3%82%BF 計算方法は左記参照
#define PICO_CLOCK_FREQ 252000000
#define CLKDIV 252
#define PWM_WRAP ((PICO_CLOCK_FREQ / (CLKDIV * 50)) - 1)

uint8 in_pins[] = { IN1, IN2, IN3, IN4, OUT1, OUT2, OUT3, OUT4, BTN, OUT5, OUT6 };
uint8 out_pins[] = { OUT1, OUT2, OUT3, OUT4, OUT5, OUT6, LED, IN1, IN2, IN3, IN4 };

void IJB_pwm(int port, int plen, int len) {
    if (!(1 <= port && port <= 6)) {
        return;
    }

    if (plen < 0) {
        plen = 0;
    } else if (plen > 2000) {
        plen = 2000;
    }
    uint8 pin = out_pins[port - 1];
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv(slice_num, CLKDIV);
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_gpio_level(pin, (PWM_WRAP + 1) * plen / 2000);
    pwm_set_enabled(slice_num, true);
}

bool is_adc_pin(uint gpio) {
    return gpio == IN1 || gpio == IN2 || gpio == BTN;
}

/*TODO プルの指定をどうするか考える
現状
IN プルアップ(デフォルト)　プルダウンにも変更可能
OUT 指定しない
ANA 指定しない
*/

//TODO ANA使うためにADCの設定(特にadc_gpio_init)が必要か確認する
//しなくても動くように見えるが...
void io_init() {
    adc_init();
    for (int i = 0; i < 4; i++) {
        uint8 pin = in_pins[i];
        if (is_adc_pin(pin)) {
            adc_gpio_init(pin);
        } else {
            gpio_init(pin);
            gpio_pull_up(pin);
        }
    }
    for (int i = 1; i <= 6; i++) {
        IJB_out(i, 0);
    }
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    // adc_gpio_init(BTN);
    gpio_init(BTN);
    gpio_pull_up(BTN);
}

//TODO 反応するキーを絞るか検討する
int IJB_btn(int n) {
    if (n < 0) {
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
    } else if (n == 0) {
        return !gpio_get(BTN);
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
    for (int i = 0; i < IO_PIN_NUM; i++) {
        bool bit = gpio_get(in_pins[i]);
        res |= bit << i;
        printf("%d\n", bit);
    }
    return res;
}

void IJB_out(int port, int st) {
    if (!(0 <= port && port <= IO_PIN_NUM)) {
        return;
    }
    if (port == 0) {
        for (int i = 0; i < IO_PIN_NUM; i++) {
            gpio_put(out_pins[i], st & (1 << i));
        }
    } else {
        uint8 pin = out_pins[port - 1];
        gpio_init(pin);
        if (st >= 0) {
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, st);
        } else if (st == -1) {
            gpio_pull_down(pin);
        } else if (st == -2) {
            gpio_pull_up(pin);
        }
    }
}

INLINE void IJB_led(int st) {
    IJB_out(7, st != 0);
}

INLINE int IJB_ana(int n) {
    if (0 <= n && n <= 2) {
        if (n == 0) {
            n = 9;
        }
        uint8 pin = in_pins[n - 1];
        adc_select_input(pin - 26);
        int v = adc_read() >> 2;//最大値を2^12から2^10に
        return v;
    } else {
        return 0;
    }
}

INLINE void IJB_clo() {
    io_init();
}