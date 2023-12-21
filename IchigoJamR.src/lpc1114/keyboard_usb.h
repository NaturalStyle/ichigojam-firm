// created by jig.jp, Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/

#ifndef __KEYBOARD_USB_H__
#define __KEYBOARD_USB_H__

/*
uint8 nkeybuf = 0;
char keybuf[KEY_BUF_LEN];
*/
#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2) // kbhitとnkeybuf分
char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！
//#define keybuf ((char*)(ram + OFFSET_RAM_KEYBUF)) // 24560+56
//volatile uint8 key_kbhit = 0; // SPC DOWN UP RIGHT LEFT // 28-32

#define keykbhit keybuf[-1]

void key_kbhit(int n) {
	keykbhit |= 1 << n;
}
void key_kbhit_rel(int n) {
	keykbhit &= ~(1 << n);
}

void key_pushc(char c) {
	/*
	if (key_nbuf < key_BUF_LEN) { // バッファ一杯ならいれない
		key_buf[key_nbuf++] = c;
	}
	*/
	if (*keybuf < KEY_BUF_LEN) {
		(*keybuf)++;
		keybuf[(uint8)*keybuf] = c;
	}
}
void key_push(char* s) { // for function keys
	for (;;) {
		char c = *s++;
		if (!c)
			break;
		key_pushc(c);
	}
}


// ps2

//#define key_BUF_LEN 10


/*
#define PS2KB_CLK 10
#define PS2KB_DATA 11
#define key_WAKEUP_IRQn WAKEUP10_IRQn
*/

#define PS2KB_CLK 7
#define PS2KB_DATA 3

#define USBKB_DP PS2KB_CLK
#define USBKB_DM PS2KB_DATA

/*
struct ps2flg_def {
	char release:1;
	char shift_r:1;
	char shift_l:1;
	char alt_l:1;
	char e0:1;
	char caps:1; // default caps_on
	char insert:1; // 1.0.2b2 1 -> 0
	char kana:1;
};
*/
struct keyflg_def key_flg;


/*
                   bss
21424	     56	   2676	  24156 before
21424	     56	   2668	  24148	after  ... key_flg の bit field化でbssが8byte減った
*/

/*

*/
uint8 PACKET_EOP[] = { 4, 0x02, 0x02, 0x08, 0x08 };

uint8 PACKET_SETUP_TOKEN_0[] = { 36,//			DATA_PKT_SETUP_TOKEN_0:
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
			0x04, 0x08, 0x08, 0x08, 0x04, 0x04, 0x08, 0x04,
			0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04,
			0x08, 0x04, 0x08, 0x04, 0x04, 0x08, 0x04, 0x08,
			0x02, 0x02, 0x08, 0x08
								 };
uint8 PACKET_SETUP_TOKEN_1[] = { 36,		// DATA_PKT_SETUP_TOKEN_1:
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
			0x04, 0x08, 0x08, 0x08, 0x04, 0x04, 0x08, 0x04,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08,
			0x02, 0x02, 0x08, 0x08
							   };

uint8 PACKET_SET_ADDRESS_1[] = { 100, //DATA_PKT_SET_ADDRESS_1:
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
0x04, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04, 0x04,
0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04,
0x04, 0x08, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04,
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
0x08, 0x08, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08,
0x08, 0x04, 0x04, 0x08, 0x04, 0x04, 0x08, 0x04,
0x02, 0x02, 0x08, 0x08
							   };

uint8 PACKET_ACK[] = { 20,
//					  DATA_PKT_ACK:
					  0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
					  0x08, 0x08, 0x04, 0x08, 0x08, 0x04, 0x04, 0x04,
					  0x02, 0x02, 0x08, 0x08
					   };
		
uint8 PACKET_SET_CONFIGURATION_1[] = { 100,
			//DATA_PKT_SET_CONFIGURATION_1:
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
			0x04, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04, 0x04,
			0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04,
			0x04, 0x08, 0x04, 0x04, 0x08, 0x04, 0x08, 0x04,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x08, 0x08, 0x08, 0x04, 0x08, 0x08, 0x04, 0x08,
			0x08, 0x04, 0x04, 0x08, 0x04, 0x04, 0x08, 0x04,
			0x02, 0x02, 0x08, 0x08
									   };

uint8 PACKET_DATA_IN_0_0[] = { 36,
							   //			DATA_PKT_DATA_IN_0_0:
							   0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
							   0x04, 0x08, 0x04, 0x04, 0x08, 0x08, 0x08, 0x04,
							   0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04,
							   0x08, 0x04, 0x08, 0x04, 0x04, 0x08, 0x04, 0x08,
							   0x02, 0x02, 0x08, 0x08
							   };

uint8 PACKET_DATA_IN_0_2[] = { 36,
							   //			DATA_PKT_DATA_IN_0_2:
0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x4, 0x4, 0x8, 0x4, 0x4, 0x8, 0x8, 0x8, 0x4, 0x8, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x4, 0x8, 0x8, 0x4, 0x4, 0x2, 0x2, 0x8, 0x8};

uint8 PACKET_DATA_IN_0_1[] = { 36,
							   0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x4, 0x4, 0x8, 0x4, 0x4, 0x8, 0x8, 0x8, 0x4, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x4, 0x8, 0x8, 0x8, 0x8, 0x2, 0x2, 0x8, 0x8};

uint8 PACKET_DATA_IN_1_0[] = { 36,
//			DATA_PKT_DATA_IN_1_0:
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
			0x04, 0x08, 0x04, 0x04, 0x08, 0x08, 0x08, 0x04,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x08,
			0x04, 0x08, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08,
			0x02, 0x02, 0x08, 0x08
						   };
uint8 PACKET_DATA_IN_1_1[] = { 36,
//			DATA_PKT_DATA_IN_1_1:
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
			0x04, 0x08, 0x04, 0x04, 0x08, 0x08, 0x08, 0x04,
			0x04, 0x08, 0x04, 0x08, 0x04, 0x08, 0x04, 0x04,
			0x08, 0x04, 0x08, 0x08, 0x08, 0x04, 0x04, 0x08,
			0x02, 0x02, 0x08, 0x08
							   };
uint8 PACKET_SOF[] = { 36,
					   0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x4, 0x4, 0x8, 0x8, 0x4, 0x8, 0x8, 0x4, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x8, 0x4, 0x4, 0x8, 0x4, 0x8, 0x2, 0x2, 0x8, 0x8
					   };

uint8 usbdebug = 0;
uint8 usbinit = 0;

void usb_sendPacket(uint8* packet) {
	LPC_GPIO0->DIR |= ((1 << USBKB_DP) | (1 << USBKB_DM)); // set out
	int len = packet[0];
	// 1.5Mbps = 666nsec単位で切り替わる、
	for (int i = 1; i <= len; i++) {
		int n = packet[i];
		LPC_GPIO0->MASKED_ACCESS[(1 << USBKB_DP) | (1 << USBKB_DM)] = (((n >> 2) & 1) << USBKB_DP) | (((n >> 3) & 1) << USBKB_DM);
		
		//			for (int i = 1; i--;) { // wait 32clock ... 1Mbps でる？
		// aitendo 4つでok ,, 6つでOK BUFFALOキーボード .... x:2つ、3つ、
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		//			}
	}
	LPC_GPIO0->DIR &= ~((1 << USBKB_DP) | (1 << USBKB_DM)); // set in
}
int usbdebugcnt = 0;

int usb_recvPacket() {
	
	while (!LPC_GPIO0->MASKED_ACCESS[(1 << USBKB_DM)]);
	
	// 受信
	int cnt = 0;
	if (usbdebug && usbdebugcnt == 0) {
		LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led on
		asm("nop");
		LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led off
	}
	/*
		キーボード
			line番号の特定番号の時に EOP 送信
			フレーム終了時にキー入力
			スクリーン乱れを防げそう、17msecごとの通信でok?
		
		アルゴリズム
		
		2種類
			特定タイミングでの0/1を見る
			変化したタイミングのカウントをとる
		
		受信バッファ bitで記録
			64bit?
			最初をLSBにしておくと、判定が楽
			タイミング合わせれば、デコードいらず？
		01列をデコード
			最初をLSBにしておくと、判定が楽
			下位byteが1かどうかでチェック可能
		
		（バッファがいらないと、メモリが節約できるけど、タイミング合わせるのが大変かも）
			ま、64bit = 4byte だからokでしょう
		
		USB参考
			http://suz-avr.sblo.jp/article/56722396.html
		
		bkdm = 1;
		cnt = 0;
		int lastt = cnt;
		int data = 0;
		int nbit = 0;
		// 最初は必ず 00000001 で始まる
		ループ
			間を少し開けて2回読んで、値が違ったらやりなおし
			if (dm == 0 && dp == 0)
				break;
			if (dm != bkdm) {
				lastt = cnt;
				data <<= 1;
				data |= dm
				nbit++;
			}
		
		DMが1から始まる
		DMとDP共に0が2回続いたら終了
		
		bkm = 1
		cnt = 0
		for (;;) {
			m = DM
			p = DP
			if (m != bkm) {
			}
	*/
	int bkm = 1;
	uint8 buf[10];
	int nbuf = 0;
	
	int savecnt = 0;
	
	// 最初の信号で同期
	int th = 3; // スレッショルドが決まる
	cnt = th / 2;
	cnt = 0; // 145
	for (;;) {
		cnt++;
		int b1 = LPC_GPIO0->MASKED_ACCESS[(1 << USBKB_DM) | (1 << USBKB_DP)];
		asm("nop");
		asm("nop");
		int b2 = LPC_GPIO0->MASKED_ACCESS[(1 << USBKB_DM) | (1 << USBKB_DP)];
		asm("nop");
		asm("nop");
		if (b1 != b2) {
			continue;
		} else if (!b1) {
			break;
		}
		/*
		int m = (b1 & (1 << USBKB_DM)) != 0;
		if (bkm != m) {
			nbuf += cnt / th;
			if ((nbuf >> 3) >= 9)
				break;
			buf[nbuf >> 3] |= 1 << (nbuf & 0x7);
			if (!savecnt)
				savecnt = cnt;
			cnt = th / 2;
		}
		*/
	}
	// 反転
	
	if (usbdebug && cnt > usbdebugcnt) {
//		put_num(savecnt);
		LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led on
		asm("nop");
		LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led off
		return cnt;
	}
//	LPC_GPIO0->DIR |= ((1 << USBKB_DP) | (1 << USBKB_DM)); // set out
	//	LPC_GPIO0->MASKED_ACCESS[(1 << USBKB_DP) | (1 << USBKB_DM)] = 1 << USBKB_DM;
	return 0;
}

inline static void key_init() {
	key_flg.release = 0;
	key_flg.shift_r = 0;
	key_flg.shift_l = 0;
	key_flg.alt_l = 0;
	key_flg.alt_r = 0;
	key_flg.ctrl_l = 0;
	key_flg.ctrl_r = 0;
	key_flg.e0 = 0;
	key_flg.caps = 1; // default caps_on
	key_flg.insert = 1; // 1.0.2b2 1 -> 0, 1.0.2b8 0 -> 1
	key_flg.kana = 0;
	
//	LPC_IOCON->SWCLK_PIO0_10 = 0xd1;
//	LPC_IOCON->R_PIO0_11 = 0xd1;
	
	
//	LPC_IOCON->PIO0_7 = 0b11010000; // pull up, key_CLK for PS/2
//	LPC_IOCON->PIO0_3 = 0b11010000; // pull up, key_DATA for PS/2
	LPC_IOCON->PIO0_7 = 0b11001000; // pull down, USB_D+? for USB
	LPC_IOCON->PIO0_3 = 0b11001000; // pull down, USB_D-? for USB
	
	/* // 元のコード、いったんHIGHにしてるけどいる？
	GPIOSetDir(0, PS2KB_CLK, 1);
	GPIOSetValue(0, PS2KB_CLK, 1);
	GPIOSetDir(0, PS2KB_CLK, 0);
	
	GPIOSetDir(0, PS2KB_DATA, 1);
	GPIOSetValue(0, PS2KB_DATA, 1);
	GPIOSetDir(0, PS2KB_DATA, 0);
	*/
	LPC_GPIO0->DIR &= ~((1 << PS2KB_CLK) | (1 << PS2KB_DATA));
	
	/*
		デバイスの接続が確認
			D-が3.3Vでロースピード（D+の場合ハイスピード）
		100msec待つ
		Reset を出力（D+/D-共に0)、最小10msec継続
		emuration（接続するデバイスの種類を固定なら省略可能）
			Get_Descriptor
			Device Descripter受け取る
		Set Address
			アドレスを1にする
		Set Configuration
			1にする
		パイプでの通信開始
		定周期でインタラプト転送を行う
	*/
	
	// LEDとボタン for debug
	LPC_GPIO1->DIR |= 1 << 5;
	LPC_GPIO1->DIR &= ~(1 << 4); // ボタンでテスト
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led on
//	while (LPC_GPIO1->MASKED_ACCESS[1 << 4]); // ボタン押されるまで待つ
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led off
	
	
	/*
	for (;;) {
		int b = LPC_GPIO0->MASKED_ACCESS[1 << USBKB_DM] != 0; // USB low speed mode 判定
//		int b = LPC_GPIO0->MASKED_ACCESS[1 << PS2KB_DATA] == 0;
//		int b = LPC_GPIO1->MASKED_ACCESS[1 << 4] != 0; // ボタンでテスト
		
//		LPC_GPIO1->MASKED_ACCESS[1 << 5] = b << 5; // LEDでUSBかどうか表示
		break;
	}
	*/
	for (int i = 48000000 / 8 / 10; i--;) { // wait 100msec
		asm("nop");
	}
	LPC_GPIO0->DIR |= ((1 << USBKB_DP) | (1 << USBKB_DM)); // set out
	LPC_GPIO0->MASKED_ACCESS[(1 << USBKB_DP) | (1 << USBKB_DM)] = 0; // reset
	for (int i = 48000000 / 8 / 100; i--;) { // wait 10msec
		asm("nop");
	}
	LPC_GPIO0->DIR &= ~((1 << USBKB_DP) | (1 << USBKB_DM)); // set in
	for (int j = 0; j < 40; j++) {
		for (int i = 48000000 / 8 / 1000; i--;) { // wait 1msec
			asm("nop");
		}
		usb_sendPacket(PACKET_EOP);
	}
	
	usb_sendPacket(PACKET_SETUP_TOKEN_0);
	usb_sendPacket(PACKET_SET_ADDRESS_1);
	usb_recvPacket(); // ack
	
//	for (int j = 0; j < 100; j++) {
		for (int i = 48000000 / 8 / 1000; i--;) { // wait 1msec // これがないと、NAKが返る
			asm("nop");
		}
		
		usb_sendPacket(PACKET_DATA_IN_0_0);
		usb_recvPacket(); // NAK or DATA1 ... AOK製、これが返ってこない
		usb_sendPacket(PACKET_ACK);
//	}
	
//	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led on
	for (int i = 48000000 / 8 / 1000; i--;) { // wait 1msec
		asm("nop");
	}
	
	usb_sendPacket(PACKET_EOP);
	usb_sendPacket(PACKET_SETUP_TOKEN_1); // TOKEN_0 ではNG
	usb_sendPacket(PACKET_SET_CONFIGURATION_1);
	usb_recvPacket(); // ack
	
	for (int i = 48000000 / 8 / 1000; i--;) { // wait 1msec // これがないと、NAKが返る
		asm("nop");
	}
	
	usbdebug = 1;
	// if err loop
	usb_sendPacket(PACKET_DATA_IN_0_1);
	usb_recvPacket(); // check the code
	usb_sendPacket(PACKET_ACK);
	
//	for (;;);
	
	for (int i = 48000000 / 8 / 10000; i--;) { // wait 0.1msec
		asm("nop");
	}
	
	/*
	LPC_GPIO0->MASKED_ACCESS[1 << USBKB_DM] = 1 << USBKB_DM; // USB low speed mode idle
	*/
	// test
//#define USB_KEYBOARD_TEST
	
	usbdebug = 1;
	usbdebugcnt = 50;
	
#ifdef USB_KEYBOARD_TEST
	for (;;) {
		usb_sendPacket(PACKET_EOP);
		usb_sendPacket(PACKET_DATA_IN_1_1);
		usb_recvPacket();
		usb_sendPacket(PACKET_ACK);
		
//		usb_sendPacket(PACKET);
		
		for (int i = 48000000 / 8 / 1000; i--;) { // wait 1msec 正しい、aitendoのは10msecでも動いた
			asm("nop");
		}
	}
#endif
	
	/*
	int cnt = 48000000 / 16;
	// 1loop 8クロック、結構ずれる・・・
	for (;;) {
		LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5;
		for (int i = cnt; i--;) {
			asm("nop");
		}
		LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0;
		for (int i = cnt; i--;) {
			asm("nop");
		}
	}
	*/
	usbdebug = 1;
	usbinit = 1;
}

void key_ping() { // 1mesc毎に呼ぶ必要あり
	if (!usbinit)
		return;
	usb_sendPacket(PACKET_EOP);
}

static void key_process(uint8 key);

int rcnt = 0;
inline static void key_tick() { // キー取得
	if (!usbinit)
		return;
	usb_sendPacket(PACKET_EOP);
	usb_sendPacket(PACKET_DATA_IN_1_1);
	rcnt = usb_recvPacket();
	usb_sendPacket(PACKET_ACK);
	if (rcnt > 0) {
		//		put_num(cnt);
		//		put_chr('\n');
		key_process(22);
	}
}
void key_tick2() { // キー取得
}

int8 key_enable_flg = 0;

inline void key_enable(uint8 b) {
	key_enable_flg = b;
}
//void uart_checker();

inline int key_btn(int n) {
//	uart_checker();
	return (keykbhit & (1 << (n - 28))) != 0;
}

/*
PS/2 keycode
	http://www.ne.jp/asahi/shared/o-family/ElecRoom/AVRMCOM/key_RS232C/KeyCordList.pdf
http://hp.vector.co.jp/authors/VA037406/html/ps2interface.htm
		// PC->KBD 仕様も！
	http://www.computer-engineering.org/ps2keyboard/scancodes2.html
		FOR US
¥xhh    16進数の文字コードを持つ文字
	\x1b = 27 ESC
*/

/*
		http://homepage2.nifty.com/k_maeda/code/ascii.html
http://okwave.jp/qa/q4045792.html
http://fukuno.jig.jp/1092
000 0000	000	0	#00	NUL	␀	^@	\0	Null文字 - 0
000 0001	001	1	#01	SOH	␁	^A		ヘッディング開始 - 真っ黒
000 0010	002	2	#02	STX	␂	^B		テキスト開始 - 灰色
000 0011	003	3	#03	ETX	␃	^C		テキスト終了 - 薄い灰色
000 0100	004	4	#04	EOT	␄	^D		伝送終了    - 濃い灰色
000 0101	005	5	05	ENQ	␅	^E		問合せ   - Q bitman -> (Q)
000 0110	006	6	06	ACK	␆	^F		肯定応答 - 人間2 - OK
000 0111	007	7	07	BEL	␇	^G	\a	ベル - BEEPにする？
000 1000	010	8	08	BS	␈	^H	\b	後退 - BS
000 1001	011	9	09	HT	␉	^I	\t	水平タブ - TAB
000 1010	012	10	0A	LF	␊	^J	\n	改行    - enter（次の行）
000 1011	013	11	0B	VT	␋	^K	\v	垂直タブ  - v-TAB
000 1100	014	12	0C	FF	␌	^L	\f	SHIFT＋SPACE 空白挿入 // 書式送り - ページ送りの意味もある これをそれ以降削除にする
000 1101	015	13	0D	CR	␍	^M	\r	復帰   - enter2（先頭に戻る） - 今は無視、ホームキーにする？
000 1110	016	14	0E	SO	␎	^N		シフトアウト（文字コード切り替え終了）  -- SHIFT+SPACE 必ず１コスペースがあく
000 1111	017	15	0F	SI	␏	^O		シフトイン（文字コード切り替え） - kana切り替え
001 0000	020	16	#10	DLE	␐	^P		伝送制御拡張 - SHIFT+Enter = 行分割?
001 0001	021	17	#11	DC1	␑	^Q		装置制御1,XON 起動 補助装置の起動 - ins切り替え
001 0010	022	18	#12	DC2	␒	^R		装置制御2 起動 - home
001 0011	023	19	#13	DC3	␓	^S		装置制御3,XOFF 停止 - page up
001 0100	024	20	#14	DC4	␔	^T		装置制御4 停止        page down
001 0101	025	21	#15	NAK	␕	^U		否定応答 - NG - LOCATE X:次のキャラクタ-32、Y:次の次のキャラクタ-32
001 0110	026	22	#16	SYN	␖	^V		同期信号 - SYNC
001 0111	027	23	#17	ETB	␗	^W		伝送ブロック終結（分割するときの末尾） - End
001 1000	030	24	#18	CAN	␘	^X		取消 - 行削除、こっちにしよう
001 1001	031	25	#19	EM	␙	^Y		媒体終端（EOFみたいなもの） -> (CC)とか
001 1010	032	26	1A	SUB	␚	^Z		置換（エラーのあったときの置換用？） -> (R)とか
001 1011	033	27	1B	ESC	␛	^[	\e	エスケープ - ESC
001 1100	034	28	1C	FS	␜	^\		ファイル分離標識 - ←
001 1101	035	29	1D	GS	␝	^]		グループ分離標識 - →
001 1110	036	30	1E	RS	␞	^^		レコード分離標識 - ↑
001 1111	037	31	1F	US	␟	^_		ユニット分離標識 - ↓
*/

#ifndef KEYLAYOUT_US
#ifndef KEYLAYOUT_JP_DVORAK
// jp-keyboard
//static const char PS2_KB[]       = "                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9  ";
//static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9  ";
// 後ろ-2 長さ128->126
static const char PS2_KB[]       = "                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9";
// -21、後ろ-2 長さ128->105 us-keyboardが切れない
//static const char PS2_KB[]       = "q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
//static const char PS2_KB_SHIFT[] = "Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9";
#else
// jp-keyboard-dvorak
//static const char PS2_KB[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '1', ' ', ' ', ' ', ';', 'o', 'a', ',', '2', ' ', ' ', 'j', 'q', 'e', '.', '4', '3', ' ', ' ', ' ', 'k', 'u', 'y', 'p', '5', ' ', ' ', 'b', 'x', 'd', 'i', 'f', '6', ' ', ' ', ' ', 'm', 'h', 'g', '7', '8', ' ', ' ', 'w', 't', 'c', 'r', '0', '9', ' ', ' ', 'v', 'z', 'n', 's', 'l', '[', ' ', ' ', '`', '-', ' ', '/', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
//static const char PS2_KB_SHIFT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '!', ' ', ' ', ' ', ':', 'O', 'A', '<', '@', ' ', ' ', 'J', 'Q', 'E', '>', '$', '#', ' ', ' ', ' ', 'K', 'U', 'Y', 'P', '%', ' ', ' ', 'B', 'X', 'D', 'I', 'F', '^', ' ', ' ', ' ', 'M', 'H', 'G', '&', '*', ' ', ' ', 'W', 'T', 'C', 'R', ')', '(', ' ', ' ', 'V', 'Z', 'N', 'S', 'L', '{', ' ', ' ', '~', '_', ' ', '?', '}', ' ', ' ', ' ', ' ', '\n', '+', ' ', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
static const char PS2_KB[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '1', ' ', ' ', ' ', ';', 'o', 'a', ',', '2', ' ', ' ', 'j', 'q', 'e', '.', '4', '3', ' ', ' ', ' ', 'k', 'u', 'y', 'p', '5', ' ', ' ', 'b', 'x', 'd', 'i', 'f', '6', ' ', ' ', ' ', 'm', 'h', 'g', '7', '8', ' ', ' ', 'w', 't', 'c', 'r', '0', '9', ' ', ' ', 'v', 'z', 'n', 's', 'l', '[', ' ', ' ', '`', '-', ' ', '/', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9' };
static const char PS2_KB_SHIFT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '!', ' ', ' ', ' ', ':', 'O', 'A', '<', '@', ' ', ' ', 'J', 'Q', 'E', '>', '$', '#', ' ', ' ', ' ', 'K', 'U', 'Y', 'P', '%', ' ', ' ', 'B', 'X', 'D', 'I', 'F', '^', ' ', ' ', ' ', 'M', 'H', 'G', '&', '*', ' ', ' ', 'W', 'T', 'C', 'R', ')', '(', ' ', ' ', 'V', 'Z', 'N', 'S', 'L', '{', ' ', ' ', '~', '_', ' ', '?', '}', ' ', ' ', ' ', ' ', '\n', '+', ' ', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9' };
#endif
#else
// us-keyboard
//static const char PS2_KB[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9  ";
//static const char PS2_KB_SHIFT[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P~  _\" {+    \n} |        \b  1 47   0.2568   +3-*9  ";
// 後ろ-2
static const char PS2_KB[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9";
static const char PS2_KB_SHIFT[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P_  _\" {+    \n} |        \b  1 47   0.2568   +3-*9";
#endif

#define IME_KEYMAP 1
#define IME_JP_KANA 2
#define IME_VN_TELEX 3

#if LANG == LANG_JP
	#define IME IME_JP_KANA
#endif

// Mongol
#if LANG == LANG_MN
	#define IME IME_KEYMAP
	static const char PS2_KB_ALT[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\xd3', '\xbe', ' ', ' ', ' ', '\xdd', '\xd9', '\xc9', '\xd4', '-', ' ', ' ', '\xc5', '\xd5', '\xc1', '\xd1', '\xa0', '"', ' ', ' ', ' ', 'c', '\xce', '\xdb', '\xc6', ':', ' ', ' ', '\xc8', '\xcc', 'x', 'a', '\xcd', '.', ' ', ' ', ' ', '\xd0', 'p', '\xc3', '_', ',', ' ', ' ', '\xda', 'o', '\xd6', '\xd2', '?', '%', ' ', ' ', '\xc2', '\xdc', '\xcb', '\xc4', '\xc7', 'e', ' ', ' ', '\\', '\xcf', ' ', '\xca', '\xd7', ' ', ' ', ' ', ' ', '\n', '\xd8', ' ', '\xd8', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\xc0', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
	static const char PS2_KB_SHIFT_ALT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\xb3', '1', ' ', ' ', ' ', '\xbd', '\xb9', '\xa9', '\xb4', '2', ' ', ' ', '\xa5', '\xb5', '\xa1', '\xb1', '4', '3', ' ', ' ', ' ', 'C', '\xae', '\xbb', '\xa6', '5', ' ', ' ', '\xa8', '\xac', 'X', 'A', '\xad', '6', ' ', ' ', ' ', '\xb0', 'P', '\xa3', '7', '8', ' ', ' ', '\xba', 'O', '\xb6', '\xb2', '0', '9', ' ', ' ', '\xa2', '\xbc', '\xab', '\xa4', '\xa7', 'E', ' ', ' ', '_', '\xaf', ' ', '\xaa', '\xb7', ' ', ' ', ' ', ' ', '\n', '\xb8', ' ', '\xb8', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
#endif


// Vietnam
#if LANG == LANG_VI
	#if 0
		#define IME IME_KEYMAP
		static const char PS2_KB_ALT[] =       "`             `      q\xc1   zsaw\xc2  cxde\xc4\xc3   vftr\xc8  nbhgy\xc9   mju\xca\xcb  ,kio\xc5\xcc  ./l;p-  \\' \xc6\xa0    \n\xc7 \\        \b  1 47   0.2568   +3-*9";
		static const char PS2_KB_SHIFT_ALT[] = "~             ~      Q\xa1   ZSAW\xa2  CXDE\xa4\xa3   VFTR\xa8  NBHGY\xa9   MJU\xaa\xab  <KIO\xa5\xac  >?L:P~  _\" \xa6\xc0    \n\xa7 |        \b  1 47   0.2568   +3-*9";
	#else
		#define IME IME_VN_TELEX
	#endif
#endif


uint8 key_kana_buf[2]; // xtsu/ltsu 対応する場合増やす必要あり
char key_kana_nbuf = 0;

//#define key_TICK systick
#define key_TICK() screen_tick()
#define key_TICK_DIF 1

static inline void key_process(uint8 key) {
	if (key > 0xe0 && key != 0xf0) // 制御コード無視、もっと広くてもいいかも
		return;
	uint8 k = 0;
	
	//		xprintf("%02x ", key);
	if (key == 0xe0) {
		key_flg.e0 = 1;
	} else if (key == 0xf0) {
		key_flg.release = 1;
	} else {
		if (key_flg.release) {
			if (key_flg.e0) {
				if (key == 0x75) { // up
					//						key_kbhit &= ~(1 << 2);
					key_kbhit_rel(2);
				} else if (key == 0x72) { // down
					//						key_kbhit &= ~(1 << 3);
					key_kbhit_rel(3);
				} else if (key == 0x6b) { // left
					//						key_kbhit &= ~(1 << 0);
					key_kbhit_rel(0);
				} else if (key == 0x74) { // right
					//						key_kbhit &= ~(1 << 1);
					key_kbhit_rel(1);
				} else if (key == 0x11) { // right ALT
					key_flg.alt_r = 0;
				} else if (key == 0x14) { // right CTRL
					key_flg.ctrl_r = 0;
				}
				key_flg.e0 = 0;
			} else {
				if (key == 0x12) { // left SHIFT
					key_flg.shift_l = 0;
				} else if (key == 0x59) { // right SHIFT
					key_flg.shift_r = 0;
				} else if (key == 0x11) { // left ALT
					key_flg.alt_l = 0;
				} else if (key == 0x14) { // left CTRL
					key_flg.ctrl_l = 0;
				} else if (key == 0x29) {
					//						key_kbhit &= ~(1 << 4);
					key_kbhit_rel(4);
				}
			}
			key_flg.release = 0;
		} else {
			if (key_flg.e0) {
				if (key == 0xf0) {
					key_flg.release = 1;
				} else {
					if (key == 0x75) { // up
						k = 30;
						//							key_kbhit |= 1 << 2;
						key_kbhit(2);
					} else if (key == 0x72) { // down
						k = 31;
						//							key_kbhit |= 1 << 3;
						key_kbhit(3);
					} else if (key == 0x6b) { // left
						k = 28;
						//							key_kbhit |= 1 << 0;
						key_kbhit(0);
					} else if (key == 0x74) { // right
						k = 29;
						//							key_kbhit |= 1 << 1;
						key_kbhit(1);
					} else if (key == 0x4A) {
						k = '/';
					} else if (key == 0x5A) {
						k = key_flg.shift_r || key_flg.shift_l ? 0x10 : '\n'; // ver1.0.1
					} else if (key == 0x71) {
						k = 0x7f;
					} else if (key == 0x6c) { // home
						k = 0x12;
					} else if (key == 0x69) { // end
						k = 0x17;
					} else if (key == 0x7d) { // page up
						k = 0x13;
					} else if (key == 0x7a) { // page down
						k = 0x14;
					} else if (key == 0x70) {
						key_flg.insert = !key_flg.insert;
					} else if (key == 0x11) { // right ALT = kana -> same as left ALT
						/*
							if (key_flg.shift_r || key_flg.shift_l) { // SHIFT + right ALT = ins
								key_flg.insert = !key_flg.insert;
							} else {
								key_flg.kana = !key_flg.kana; // right ALT = kana
								key_kana_nbuf = 0;
							}
							*/
						key_flg.alt_r = 1;
						if (key_flg.ctrl_r || key_flg.ctrl_l) { // INS
							key_flg.insert = !key_flg.insert;
						} else {
							key_flg.kana = !key_flg.kana; // right ALT = kana
							key_kana_nbuf = 0;
						}
					} else if (key == 0x14) { // right CTRL
						key_flg.ctrl_r = 1;
					}
					key_flg.e0 = 0;
				}
			} else {
				if (key == 0x12) { // left SHIFT
					key_flg.shift_l = 1;
					if (key_flg.ctrl_r || key_flg.ctrl_l) { // lang
						key_flg.kana = !key_flg.kana;
						key_kana_nbuf = 0;
					}
				} else if (key == 0x59) { // right SHIFT
					key_flg.shift_r = 1;
					if (key_flg.ctrl_r || key_flg.ctrl_l) { // lang
						key_flg.kana = !key_flg.kana;
						key_kana_nbuf = 0;
					}
				} else if (key == 0x58) { // caps
					if (key_flg.kana) {
						key_flg.kana = 0; // 0.9.4
						key_kana_nbuf = 0;
					} else {
						key_flg.caps = !key_flg.caps;
						//							key_send_caps(key_flg.caps); // 割り込みの関係で微妙
					}
				} else if (key == 0x13) { // kana
					key_flg.kana = !key_flg.kana;
					key_kana_nbuf = 0;
				} else if (key == 0x11) { // left ALT
					key_flg.alt_l = 1;
					if (key_flg.ctrl_r || key_flg.ctrl_l) { // INS
						key_flg.insert = !key_flg.insert;
					}
				} else if (key == 0x14) { // left CTRL
					key_flg.ctrl_l = 1;
				} else if (key == 0x29) { // space
					//						k = 0x20;
					k = key_flg.shift_r || key_flg.shift_l ? 0xe : 0x20; // Shift+Space=空白挿入
					//						key_kbhit |= 1 << 4;
					key_kbhit(4);
				} else if (key == 0x5A) { // enter
					//k = '\n';
					k = key_flg.shift_r || key_flg.shift_l ? 0x10 : '\n'; // Shift+Enter=行分割 ver1.0.0b14
					key_kana_nbuf = 0; // 1.2.1
				} else if (key == 0x05) { // F1
					//						key_push("\030CLS\n"); // 022(8進数) = 0x12
					key_push("\x13\x0c"); // page up, clear after cursor 1.2b13
				} else if (key == 0x06) { // F2
					key_push("\x18LOAD");
				} else if (key == 0x04) { // F3
					key_push("\x18SAVE");
				} else if (key == 0x0c) { // F4
					//						key_push("\x18\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
					key_push("\x18\030LIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
				} else if (key == 0x03) { // F5
					key_push("\x18RUN\n");
				} else if (key == 0x0B) { // F6
					key_push("\x18?FREE()\n");
				} else if (key == 0x83) { // F7
					key_push("\x18OUT0\n"); // 0.9.4
				} else if (key == 0x0A) { // F8
					key_push("\x18VIDEO1\n"); // 0.9.6
				} else if (key == 0x01) { // F9
					key_push("\x18\014FILES\n"); // 1.0.0b14 // \014(8進数) = 0x0c
				} else if (key == 0x09) { // F10
					key_pushc(0x18); // 0.9.9
				} else if (key == 0x78) { // F11
					key_pushc(0x0c); // 0.9.9
				} else if (key == 0x07) { // F12
					key_pushc(0x10); // 1.0.0b14
					// F7-F12:  83 0A 01 09 78 07
				} else if (key == 0x76) { // ESC
					k = 0x1b;
					key_kana_nbuf = 0; // 1.2.1
				} else if (key == 0x0d) { // TAB
					key_pushc('\t');
				} else if (key == 0x7e) { // Scroll Lcok 無視、小型キーボードで起動時に送られてくる
				} else if (key == 0x77) { // num lock 無視
				} else if (key == 0xe1) { // pause の一部 無視 (e1, 14, 77, e1, f0, 14, f0, 77)
				} else if (key == 0x14) { // 左CTRL 無視
#ifndef KEYLAYOUT_US
				} else if (key == 0x0e) { // 半角/全角 無視
#endif
				} else if (key == 0x64) { // 変換 無視
				} else if (key == 0x67) { // 無変換 無視
				} else {
					int shift = key_flg.shift_r || key_flg.shift_l;
					//						if (key < 0x80) {
					if (key < 0x80 - 2) { // 1.2b6
						//						if (key >= 21 && key < 0x80 - 2) { // 1.2b6 x us非対応
						//							if (key_flg.alt_l || key_flg.alt_r) {
						if (key_flg.alt_l) { // やっぱり左だけにする 1.1b7
							// shift でかえる 0-F + G-V で32キャラ x 2
							k = PS2_KB[key];
							//			_printf("%x %d\n", key, k);
							if (k >= '0' && k <= '9') {
								k = k - '0' + (shift ? 0x80 : 0xe0);
							} else if (k >= 'a' && k <= 'v') {
								k = k - ('a' - 10) + (shift ? 0x80 : 0xe0);
							} else if (k == '[') { // ver 1.2b5
								k = '_';
							} else if (k == ']') { // ver 1.2b5
								k = key_flg.kana ? 0xa0 : '\\';
							} else {
								k = 0;
							}
						} else if (!key_flg.kana) {
							if (shift) {
								k = PS2_KB_SHIFT[key];
								if (key_flg.caps) {
									if (k >= 'A' && k <= 'Z') {
										k += 'a' - 'A';
									}
								}
							} else {
								k = PS2_KB[key];
								if (key_flg.caps) {
									if (k >= 'a' && k <= 'z') {
										k += 'A' - 'a';
									}
								}
							}
						} else { // kana
#if IME == IME_KEYMAP
							if (shift) {
								k = PS2_KB_SHIFT_ALT[key];
								/*
									if (key_flg.caps) {
										if (k >= 'A' && k <= 'Z') {
											k += 'a' - 'A';
										}
									}
									*/
							} else {
								k = PS2_KB_ALT[key];
								/*
									if (key_flg.caps) {
										if (k >= 'a' && k <= 'z') {
											k += 'A' - 'a';
										}
									}
									*/
							}
#elif IME == IME_VN_TELEX
							if (shift) {
								k = PS2_KB_SHIFT[key];
							} else {
								k = PS2_KB[key];
							}
							if (key_kana_nbuf == 1) {
								int bk = key_kana_buf[0];
								// 小文字の場合
								if (bk == 'a' || bk == 'i' || bk == 'u' || bk == 'e' || bk == 'o' || bk == 'y' || (bk >= 0xc1 && bk <= 0xc7 && bk != 0xc5) ||
									bk == 'A' || bk == 'I' || bk == 'U' || bk == 'E' || bk == 'O' || bk == 'Y' || (bk >= 0xa1 && bk <= 0xa7 && bk != 0xa5)) { // 母音
									if (k == 's' || k == 'S') { // '
										k = 0xcb;
									} else if (k == 'f' || k == 'F') { // `
										k = 0xc8;
									} else if (k == 'r' || k == 'R') { // ?
										k = 0xc9;
									} else if (k == 'x' || k == 'X') { // ~
										k = 0xca;
									} else if (k == 'j' || k == 'J') { // .
										k = 0xcc;
									} else if (bk == 'a' && k == 'w') { // au
										key_pushc(0x08); // backspace
										k = 0xc1;
									} else if (bk == 'a' && k == 'a') { // a^
										key_pushc(0x08); // backspace
										k = 0xc2;
									} else if (bk == 'e' && k == 'e') { // e^
										key_pushc(0x08); // backspace
										k = 0xc3;
									} else if (bk == 'u' && k == 'w') { // u'
										key_pushc(0x08); // backspace
										k = 0xc6;
									} else if (bk == 'o' && k == 'w') { // o'
										key_pushc(0x08); // backspace
										k = 0xc7;
									} else if (bk == 'A' && (k == 'w' || k == 'W')) { // Au
										key_pushc(0x08); // backspace
										k = 0xa1;
									} else if (bk == 'A' && (k == 'a' || k == 'A')) { // A^
										key_pushc(0x08); // backspace
										k = 0xa2;
									} else if (bk == 'E' && (k == 'e' || k == 'E')) { // E^
										key_pushc(0x08); // backspace
										k = 0xa3;
									} else if (bk == 'U' && (k == 'w' || k == 'W')) { // U'
										key_pushc(0x08); // backspace
										k = 0xa6;
									} else if (bk == 'O' && (k == 'w' || k == 'W')) { // O'
										key_pushc(0x08); // backspace
										k = 0xa7;
									}
								} else if (bk == 'd' && k == 'd') { // dd
									key_pushc(0x08); // backspace
									k = 0xc5;
								} else if (bk == 'D' && (k == 'd' || k == 'D')) { // dd
									key_pushc(0x08); // backspace
									k = 0xa5;
								}
							}
							// http://www.vntyping.com/
							key_kana_buf[0] = k;
							key_kana_nbuf = 1;
							
#elif IME == IME_JP_KANA
							if (shift) {
								k = PS2_KB_SHIFT[key];
								if (key_flg.caps) {
									if (k >= 'A' && k <= 'Z') {
										k += 'a' - 'A';
									}
								}
							} else {
								k = PS2_KB[key];
								int m = -1;
								// 0xb1 a
								if (k == 'a') {
									m = 0;
								} else if (k == 'i') {
									m = 1;
								} else if (k == 'u') {
									m = 2;
								} else if (k == 'e') {
									m = 3;
								} else if (k == 'o') {
									m = 4;
								} else if (k == '.') {
									k = 0xa1;
								} else if (k == ',') {
									k = 0xa4;
								} else if (k == '-') {
									k = 0xb0;
								} else if (k == '[') {
									k = 0xa2;
								} else if (k == ']') {
									k = 0xa3;
								} else if (k == '/') {
									k = 0xa5;
								} else if (k == '\\') {
									k = 0xa0; // yen mark
								} else if (k >= 'a' && k <= 'z') {
									if (key_kana_nbuf == 2) {
										//k = key_kana_buf[0];
										key_kana_buf[0] = key_kana_buf[1];
										key_kana_buf[1] = k;
										k = 0;
										/*
											key_kana_nbuf = 0;
											k = 0;
											*/
									} else if (key_kana_nbuf == 1) {
										if (key_kana_buf[0] == 'n') { // ん の簡易入力 復活 1.2.1
											if (k == 'n') {
												key_kana_nbuf = 0;
											} else {
												key_kana_buf[0] = k;
											}
											k = 0xdd;
										} else if (key_kana_buf[0] == k) {
											//												k = 0xaf;
											if (k == 'n') {
												k = 0xdd;
												key_kana_nbuf = 0;
											} else {
												k = 0xaf;
											}
										} else {
											key_kana_buf[(int)key_kana_nbuf] = k;
											key_kana_nbuf++;
											k = 0;
										}
									} else {
										key_kana_buf[(int)key_kana_nbuf] = k;
										key_kana_nbuf++;
										k = 0;
									}
								}
								if (m >= 0) {
									if (key_kana_nbuf == 0) {
										k = 0xb1 + m;
									} else if (key_kana_nbuf == 1) {
										switch (key_kana_buf[0]) {
										  case 'k':
											k = 0xb6 + m;
											break;
										  case 's':
											k = 0xbb + m;
											break;
										  case 't':
											k = 0xc0 + m;
											break;
										  case 'n':
											k = 0xc5 + m;
											break;
										  case 'h':
											k = 0xca + m;
											break;
										  case 'm':
											k = 0xcf + m;
											break;
										  case 'y':
											if (m == 1) {
												k = 0xb2;
											} else if (m == 3) {
												k = 0xb4;
											} else {
												k = 0xd4 + m / 2;
											}
											break;
										  case 'r':
											k = 0xd7 + m;
											break;
										  case 'w':
											if (m == 0) {
												k = 0xdc;
											} else if (m == 1) {
												key_pushc(0xb3);
												k = 0xa8;
											} else if (m == 2) {
												k = 0xb3;
											} else if (m == 3) {
												key_pushc(0xb3);
												k = 0xaa;
											} else if (m == 4) {
												k = 0xa6;
											}
											break;
										  case 'l':
										  case 'x':
											k = 0xa7 + m;
											break;
										  case 'g':
											key_pushc(0xb6 + m);
											k = 0xde;
											break;
										  case 'z':
											key_pushc(0xbb + m);
											k = 0xde;
											break;
										  case 'j': // じゃじじゅじぇじょ
											key_pushc(0xbc);
											key_pushc(0xde);
											if (m == 0) {
												k = 0xac;
											} else if (m == 1) {
												k = 0;
											} else if (m == 2) {
												k = 0xad;
											} else if (m == 3) {
												k = 0xaa;
											} else if (m == 4) {
												k = 0xae;
											}
											break;
										  case 'f': // ふぁふぃふふぇふぉ
											key_pushc(0xcc);
											if (m == 2) {
												k = 0;
											} else {
												k = 0xa7 + m;
											}
											break;
										  case 'v': // ヴぁヴぃヴヴぇヴぉ
											key_pushc(0xb3);
											key_pushc(0xde); // 濁点 1.2.1
											if (m == 2) {
												k = 0;
											} else {
												k = 0xa7 + m;
											}
											break;
										  case 'd':
											key_pushc(0xc0 + m);
											k = 0xde;
											break;
										  case 'b':
											key_pushc(0xca + m);
											k = 0xde;
											break;
										  case 'p':
											key_pushc(0xca + m);
											k = 0xdf;
											break;
										}
										key_kana_nbuf = 0;
									} else if (key_kana_nbuf == 2) {
										char k0 = key_kana_buf[0];
										char k1 = key_kana_buf[1];
										if ((k0 == 'c' || k0 == 's') && k1 == 'h') { // ちゃちちゅちぇちょ、しゃししゅしぇしょ(1.2.1)
											key_pushc(k0 == 'c' ? 0xc1 : 0xbc);
											if (m == 0) {
												k = 0xac;
											} else if (m == 1) {
												k = 0;
											} else if (m == 2) {
												k = 0xad;
											} else if (m == 3) {
												k = 0xaa;
											} else if (m == 4) {
												k = 0xae;
											}
										} else if (k0 == 't' && k1 == 's' && m == 2) {
											k = 0xc2;
										} else if ((k0 == 'l' || k0 == 'x') && k1 == 't' && m == 2) { // ltu xtu っ
											k = 0xaf;
										} else if ((k0 == 't' || k0 == 'd') && k1 == 'h') {
											key_pushc(0xc3);
											if (k0 == 'd')
											key_pushc(0xde); // 濁点
											if (!(m & 1)) {
												k = 0xac + m / 2;
											} else {
												k = 0xa7 + m;
											}
										} else if (k1 == 'y') {
											switch (k0) {
											  case 'k': // きゃきゅきょ
												key_pushc(0xb6 + 1);
												break;
											  case 's':
												key_pushc(0xbb + 1);
												break;
											  case 't':
												key_pushc(0xc0 + 1);
												break;
											  case 'n':
												key_pushc(0xc5 + 1);
												break;
											  case 'h':
												key_pushc(0xca + 1);
												break;
											  case 'f': // 1.2.1 ふゃふゅふょ
												key_pushc(0xca + 2);
												break;
											  case 'j': // 1.2.1 じゃじゅじょ
												key_pushc(0xbc);
												key_pushc(0xde);
												break;
											  case 'm':
												key_pushc(0xcf + 1);
												break;
											  case 'r':
												key_pushc(0xd7 + 1);
												break;
											  case 'g':
												key_pushc(0xb6 + 1);
												key_pushc(0xde);
												break;
											  case 'z':
												key_pushc(0xbb + 1);
												key_pushc(0xde);
												break;
											  case 'd':
												key_pushc(0xc0 + 1);
												key_pushc(0xde);
												break;
											  case 'b':
												key_pushc(0xca + 1);
												key_pushc(0xde);
												break;
											  case 'p':
												key_pushc(0xca + 1);
												key_pushc(0xdf);
												break;
											}
											if (!(m & 1)) {
												k = 0xac + m / 2;
											} else {
												k = 0xa7 + m;
											}
										}
										key_kana_nbuf = 0;
									}
								}
							}
#endif
						}
					}
				}
			}
			if (k) {
				key_pushc(k);
				if (k == 27) {
					_g.key_flg_esc = 1;
				}
			}
		}
	}
}

void key_process_usb(uint8 key) {
	if (!key_enable_flg)
		return;
	
	key_process(key);
}

// uart

#define RS485_ENABLED		0
#define TX_INTERRUPT		0		/* 0 if TX uses polling, 1 interrupt driven. */
#define MODEM_TEST			0

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

//#define BUFSIZE		0x40
//#define BUFSIZE		10 // 16->10 // ver 1.0.2b2

/* RS485 mode definition. */
#define RS485_NMMEN		(1 << 0)
#define RS485_RXDIS		(1 << 1)
#define RS485_AADEN		(1 << 2)
#define RS485_SEL		(1 << 3)
#define RS485_DCTRL		(1 << 4)
#define RS485_OINV		(1 << 5)


//volatile uint8_t UARTBuffer[BUFSIZE];
//volatile uint8_t UARTCount = 0;

//uint8 firstuart = 1; // 行頭コメントモードの名残

void UART_IRQHandler(void) { // UARTStatus 無視！
	uint8_t IIRValue = (LPC_UART->IIR >> 1) & 7; // check interrupt identification
	if ((IIRValue == IIR_RLS && (LPC_UART->LSR & LSR_RDR)) || IIRValue == IIR_RDA) { // receive data
		char c = LPC_UART->RBR;
		
		// キーフラグ シリアルからはトグルする
		if (c >= 28 && c <= 32) {
			keykbhit ^= 1 << (c - 28);
		} else if (c == 27) {
//			_g.key_flg_esc = 1;
			_g.key_flg_esc = _g.uartmode_rxd != 2; // 1.1b14
		}
		if (_g.uartmode_rxd) {
				/*
			if (firstuart) {
				if (_g.uartmode_rxd == 2) {
					key_pushc('\'');
				}
				firstuart = 0;
			}
				*/
			key_pushc(c);
//			firstuart = c == '\n';
		}
	}
}

void UARTInit(uint32_t baudrate) { // inlineにすると警告？
	NVIC_DisableIRQ(UART_IRQn);
	
	/*
	LPC_IOCON->PIO1_6 &= ~0x07;    //   UART I/O
	LPC_IOCON->PIO1_6 |= 0x01;     // UART RXD
	LPC_IOCON->PIO1_7 &= ~0x07;	
	LPC_IOCON->PIO1_7 |= 0x01;     // UART TXD
	*/
	LPC_IOCON->PIO1_6 = 0b11010001; // 0xd1 = RXD pullup
	LPC_IOCON->PIO1_7 = 0b11010001; // 0xd1 = TXD pullup
//	LPC_IOCON->PIO1_6 = 0b11000001; // 0xc1 = RXD open ... ノイズのる
//	LPC_IOCON->PIO1_7 = 0b11000001; // 0xc1 = TXD open
	
	// enable UART
	LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 12;
	LPC_SYSCON->UARTCLKDIV = 1;     /* divided by 1 */
	
	LPC_UART->LCR = 0x83;             /* 8 bits, no Parity, 1 Stop bit */
	//	uint32_t div = (((SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV) / LPC_SYSCON->UARTCLKDIV) / 16) / baudrate;
	int32_t div = ((((int)SystemCoreClock * (int)LPC_SYSCON->SYSAHBCLKDIV) / (int)LPC_SYSCON->UARTCLKDIV) / 16) / (int)baudrate;
	
	LPC_UART->DLM = div >> 8; // div / 256; ??
	LPC_UART->DLL = div & 0xff; // div % 256; ??
	LPC_UART->LCR = 0x03;		/* DLAB = 0 */
	LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
	
	/* Read to clear the line status. */
//	regVal = LPC_UART->LSR; // 1.1b14 不要？
	
	/* Ensure a clean start, no data in either TX or RX FIFO. */
	// CodeRed - added parentheses around comparison in operand of &
	while ((LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT));
	while (LPC_UART->LSR & LSR_RDR) {
//		regVal = LPC_UART->RBR;	/* Dump data from RX FIFO */ // 1.1b14 不要？
	}
	
	// Enable the UART Interrupt
	NVIC_EnableIRQ(UART_IRQn);
	NVIC_SetPriority(UART_IRQn, 0);
	LPC_UART->IER = IER_RBR | IER_RLS;	// Enable UART interrupt
}

int key_getKey() {
	if (!*keybuf) {
		return -1; // 1.2b19
	}
	int res = keybuf[1];
	(*keybuf)--;
	for (uint8 i = 0; i < *keybuf; i++)
		keybuf[i + 1] = keybuf[i + 2];
	return res;
}
void key_clearKey() {
//	key_kbhit = 0; // ver1.2b5 clear しないように変更
	*keybuf = 0;
}

#endif
