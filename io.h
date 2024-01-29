//TODO 反応するキーを絞るか検討する
int IJB_btn(int n) {
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

int IJB_in() {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_in");
}

void IJB_out(int port, int st) {
    // pico sdk / picodvi api を叩いて作っていく
    printf("ijb_out");
}
