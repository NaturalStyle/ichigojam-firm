//EEPROMのsave/loadをテストする
//int main()の if (ch < 0) {}の中で呼ぶと、EEPROMのプログラムが書き換わる
//その後LRUN100して、100~227までの数字が画面に表示されたあとFile Errorになれば正常
void eeprom_test() {
    static int i = 100;
    if (i <= 227) {
        char str[4];
        sprintf(str, "%d", i);
        key_push("10 I=");
        key_push(str);
        key_push("\n20 ?I");
        key_push("\n30 WAIT30");
        key_push("\n40 LRUN I+1");
        key_push("\nSAVE ");
        key_push(str);
        key_push("\n");
        sleep_ms(10);
        i++;
    }
}