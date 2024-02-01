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
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_in");
}

void IJB_out(int port, int st) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_out");
}
