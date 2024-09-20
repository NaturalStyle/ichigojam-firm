# オリジナルのIchigoJamとの差異
* WAIT -240でWAIT1と同じ時間,VIDEO0や数2を指定して画面出力が止まっているときは数1のマイナス指定不可
* BTN 数にSPACE,X以外のASCIIコードも指定可能に
* SAVE,LOAD 本体の保存領域が0~99まで使用可能
* TICK 数に1指定で1/(60*240)秒で1進む時間
* OUT IN3もプルの指定を可能に
* IN IN1~4の初期状態はすべてプルアップ
* ANA 0~2(BTN,IN1,IN2)のみ使用可能
* PWM 数3の指定不可
* SWITCH 非対応