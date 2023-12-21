// created by jig.jp, Taisuke Fukuno, @taisukef, http://fukuno.jig.jp/

#ifndef __KEYBOARD_PS2_H__
#define __KEYBOARD_PS2_H__

#include "../stddef.h"
#include "../lang.h"
#include "../vars.h"
#include "../ram.h"
#include "../screen.h"
#include "LPC11xx.h"

/*
uint8 nkeybuf = 0;
char keybuf[KEY_BUF_LEN];
*/
#define KEY_BUF_LEN (SIZE_RAM_KEYBUF - 2) // kbhitとnkeybuf分

// constポインタにすると容量増
//char* const keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！
//char* keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1)); // kbhit[-1], len:[0], buf:[1-(KEY_BUF_LEN-1] // 24512+60 // 小さい！
char* keybuf; // key_init() へ keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1));

//#define keybuf ((char*)(ram + OFFSET_RAM_KEYBUF)) // 24560+56
//volatile uint8 keykbhit = 0; // SPC DOWN UP RIGHT LEFT // 28-32
#define keykbhit keybuf[-1]

INLINE void key_kbhit(int n) {
	keykbhit |= 1 << n;
}
INLINE void key_kbhit_rel(int n) {
	keykbhit &= ~(1 << n);
}

static void key_pushc(char c) {
	/*
	if (key_nbuf < PS2_BUF_LEN) { // バッファ一杯ならいれない
		key_buf[key_nbuf++] = c;
	}
	*/
	if (*keybuf < KEY_BUF_LEN) {
		(*keybuf)++;
		keybuf[(uint8)*keybuf] = c;
	}
}
static void key_push(const char* s) { // for function keys
	for (;;) {
		char c = *s++;
		if (!c)
			break;
		key_pushc(c);
	}
}

// ps2

//#define PS2_BUF_LEN 10


/*
#define PS2KB_CLK 10
#define PS2KB_DATA 11
#define PS2_WAKEUP_IRQn WAKEUP10_IRQn
*/

#define PS2KB_CLK 7 // 0_7 wakeup KBD1
#define PS2KB_DATA 3 // 0_3 KBD2
#define PS2_WAKEUP_IRQn WAKEUP7_IRQn

/*
struct ps2flg_def { -> screen.h
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
0xDE 0x07 {bit0, bit1, bit2} キーボードのLEDつける

参考
http://ioiodesu.web.fc2.com/PS2/PS2.HTML

キーボードはCLKを10mS以下の間隔で監視しシステムからのコマンドに応答する。
キーボードはシステムからのコマンドに20mS以内に応答する。
システムはキーボードの応答が無効またはエラーなら再送信する
*/
#define PS2_TIMEOUT_BIT1 10 // こっちはすぐ？
#define PS2_TIMEOUT_BIT2 15 // 32000あれば十分だろう

static void inline key_send(int data) {
	volatile uint32* ps2clk = (uint32*)LPC_GPIO0->MASKED_ACCESS + (1 << PS2KB_CLK); // 1.2b61
	volatile uint32* ps2data = (uint32*)LPC_GPIO0->MASKED_ACCESS + (1 << PS2KB_DATA); // 1.2b61
	
	LPC_SYSCON->STARTERP0 &= ~(1 << PS2KB_CLK); // disable
	
//	int t1, t2, t3;
	int timeout;
	for (timeout = 0; !*ps2clk; timeout++) {
		if (timeout >> PS2_TIMEOUT_BIT1)
			return;
	}
//	t1 = timeout;
	
	LPC_GPIO0->DIR |= 1 << PS2KB_CLK; // キーボード通信禁止
	*ps2clk = 0;
	
//	for (int i = 0; i < 10; i++) video_waitSync(); // 100usec wait
	
	LPC_GPIO0->DIR |= 1 << PS2KB_DATA;
	*ps2data = 0; // start bit = 0
	LPC_GPIO0->DIR &= ~(1 << PS2KB_CLK); // キーボード通信許可
	
	for (timeout = 0; !*ps2clk; timeout++) {
		if (timeout >> PS2_TIMEOUT_BIT1)
			return;
	}
//	t2 = timeout;
	for (timeout = 0; *ps2clk; timeout++) {
		if (timeout >> PS2_TIMEOUT_BIT2) { // clock によってかえるべき？
//			xprintf("timeout!\n");
			return;
		}
	}
//	t3 = timeout; // 最大で7200くらい
	
	int cnt = 0;
	for (int i = 0; i < 8; i++) {
		int bit = data & (1 << i);
		*ps2data = bit ? ~0 : 0;
		if (bit)
			cnt++;
		while (!*ps2clk);
		while (*ps2clk);
	}
	*ps2data = cnt & 1 ? 0 : ~0; // parity
	while (!*ps2clk);
	while (*ps2clk);
	
	*ps2data = ~0; // stop bit = 1
	while (!*ps2clk);
	while (*ps2clk);
	
	LPC_GPIO0->DIR &= ~(1 << PS2KB_DATA);
	while (!*ps2data); // wait data
	while (!*ps2clk); // wait clk
	while (*ps2data); // wait data
	while (*ps2clk); // wait clk
	
	LPC_SYSCON->STARTERP0 |= 1 << PS2KB_CLK; // enable
	
//	xprintf("timeout %d %d %d %d\n", t1, t2, t3, 1 << PS2_TIMEOUT_BIT);
}

static inline void key_send_caps(int on) {
//	for (int i = 0; i < 10; i++) video_waitSync();
//	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led1
	key_send(0xed);
//	for (int i = 0; i < 10; i++) video_waitSync();
	key_send(on ? 0b100 : 0); // 2:caps, 1:num, 0:scroll
//	for (int i = 0; i < 10; i++) video_waitSync();
}
static inline void key_send_reset() {
	key_send(0xff); // reset
	//key_send_caps(1); // 入らない
}
void key_send_test() {
	// PS/AT の初期化コマンド 0FFh,0F2h,0EDh,00h,0F2hだった
	
	/*
	パソコンからキーボードへ
	ED bxxx LED
	F2 ID要求 -> ACK ('L') ('H') (elecom:56 06)
	FF リセット、セルフテスト？ -> aa
	
キーボードがパソコンへ発行するコマンドコード
00 Overrun	16バイトの受信バッファを超えた
AA BAT Completion	BAT(Basic Assurance Test)が正しく終了した
FC BAT Failed	BATが正しく終了できなかった
EE ECHO Responce	エコーコマンドへの応答を示す
F0 Break Code Prefix	Breakコードのプリフィックス
FA Ack	ECHO、RESEND以外のコマンドに対する正しく受け取られた応答メッセージ
FD Disgnostic Failure	セルフチェック時にエラー発生
FE Resend	パソコンからの送信データ再送要求
	*/
	key_send(0xff); // reset
	/*
	for (int i = 0; i < 10; i++) video_waitSync();
	for (int i = 0; i < 10; i++) video_waitSync();
	key_send(0xed);
	for (int i = 0; i < 10; i++) video_waitSync();
	key_send(0b100); // 2:caps, 1:num, 0:scroll
	*/
	
//	for (int i = 0; i < 10; i++) video_waitSync();
//	key_send(0xf2); // request ID
//	for (int i = 0; i < 10; i++) video_waitSync();
	
	/*
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	for (int i = 0; i < 10; i++) video_waitSync();
	
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led1
	for (int i = 0; i < 10; i++) video_waitSync();
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	for (int i = 0; i < 10; i++) video_waitSync();
	
	
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led1
	for (int i = 0; i < 10; i++) video_waitSync();
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	for (int i = 0; i < 10; i++) video_waitSync();
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5; // led1
	for (int i = 0; i < 10; i++) video_waitSync();
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	
	LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0; // led0
	for (int i = 0; i < 10; i++) video_waitSync();
	
	
	for (;;);
	for (int i = 0;; i++) {
//		if (LPC_GPIO0->MASKED_ACCESS[1 << 11])
		if (i % 10 > 5) { // LPC_GPIO0->MASKED_ACCESS[1 << 10])
			LPC_GPIO1->MASKED_ACCESS[1 << 5] = 1 << 5;
		} else {
			LPC_GPIO1->MASKED_ACCESS[1 << 5] = 0;
		}
		for (int i = 0; i < 10; i++) video_waitSync();
	}
	*/
}

/*
                   bss
21424	     56	   2676	  24156 before
21424	     56	   2668	  24148	after  ... key_flg の bit field化でbssが8byte減った
*/
static inline void key_init() {
	keybuf = (char*)(ram + (OFFSET_RAM_KEYBUF + 1));
	
	/*
	key_flg.release = 0;
	key_flg.shift_r = 0;
	key_flg.shift_l = 0;
	key_flg.alt_l = 0;
	key_flg.alt_r = 0;
	key_flg.ctrl_l = 0;
	key_flg.ctrl_r = 0;
	key_flg.e0 = 0;
	key_flg.kana = 0;
	key_flg.caps = 0; // default caps_on = 0 1.2b45
	key_flg.insert = 0; // 1.0.2b2 1 -> 0, 1.0.2b8 0 -> 1 / insert on = 0 1.2b45
	*/
	
//	LPC_IOCON->PIO0_7 = 0xd0; // pull up 0b11010000 (default なので省略)
//	LPC_IOCON->PIO0_3 = 0xd0; // pull up 0b11010000 (default なので省略）
	
	
	/* // 元のコード、いったんHIGHにしてるけどいる？
	GPIOSetDir(0, PS2KB_CLK, 1);
	GPIOSetValue(0, PS2KB_CLK, 1);
	GPIOSetDir(0, PS2KB_CLK, 0);
	
	GPIOSetDir(0, PS2KB_DATA, 1);
	GPIOSetValue(0, PS2KB_DATA, 1);
	GPIOSetDir(0, PS2KB_DATA, 0);
	*/
	LPC_GPIO0->DIR &= ~((1 << PS2KB_CLK) | (1 << PS2KB_DATA));
//	LPC_GPIO0->MASKEDACCESS[1 << 0] = 0;
	
	// GPIO interrupt
//	LPC_SYSCON->STARTAPRP0 &= ~(1 << PS2KB_CLK); // falling edge // bug on Breadboad
	LPC_SYSCON->STARTAPRP0 |= 1 << PS2KB_CLK; // rising edge
	LPC_SYSCON->STARTERP0 |= 1 << PS2KB_CLK; // enable
	
	NVIC_EnableIRQ(PS2_WAKEUP_IRQn);
	NVIC_SetPriority(PS2_WAKEUP_IRQn, 0);
}

#define PS2_CLK() LPC_GPIO0->MASKED_ACCESS[1 << PS2KB_CLK]
#define PS2_DATA() LPC_GPIO0->MASKED_ACCESS[1 << PS2KB_DATA]

//int key_bkt = 0; // 32bitは使いすぎ？
uint16 key_bkt;
//#define key_bkt _g.key_bk_g // varに入れても変わらない

//uint8 key_data; // varにいれると大きくなる
//char key_nbit; // varにいれると大きくなる
//char key_cnt; // PS2信号の1の数を数える、varにいれると大きくなる

#define key_data _g.key_data_g
#define key_nbit _g.key_nbit_g
#define key_cnt _g.key_cnt_g

//char key_enable_flg = 0; // varに入れたほうが大きくなる

/*
volatile char key_buf[PS2_BUF_LEN]; // ローマ時が最大３文字
volatile uint8 key_nbuf = 0;
//int key_debug = 0;
*/

static inline void key_enable(uint8 b) {
//	key_enable_flg = b;
	key_flg.enable = b;
}
//void uart_checker();

static inline int key_btn(int n) {
//	uart_checker();
//	return (keykbhit & (1 << (n - 28))) != 0;

//	int shift = n == 10 ? 5 : 28; // enter or not
	n -= n == 88 ? 88 - 5 : 28; // X or not
	return (keykbhit & (1 << n)) != 0;
}

/*
PS/2 keycode
	http://www.ne.jp/asahi/shared/o-family/ElecRoom/AVRMCOM/PS2_RS232C/KeyCordList.pdf
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

#include "keyboard_ps2_map.h"

#if KEY_LAYOUT == KEY_LAYOUT_JP
// jp-keyboard

#define PS2_KB (PS2_KB_MAP_JP + 2)
#define PS2_KB_SHIFT (PS2_KB_MAP_JP + 2 + 0x7f)


/*static const char PS2_KB_MAP[] =
	"\0\0"
	"                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9"
	"                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b  +3-*9";
	*/
/*
//static const char PS2_KB[]       = "                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9  ";
//static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9  ";
// 後ろ-2 長さ128->126
static const char PS2_KB[]       = "\0\0" "                     q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
//static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9";
static const char PS2_KB_SHIFT[] = "                     Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b  +3-*9"; // 後ろおかしかった？ 1.3.2b12
*/

// -21、後ろ-2 長さ128->105 us-keyboardが切れない
//static const char PS2_KB[]       = "q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
//static const char PS2_KB_SHIFT[] = "Q!   ZSAW\"  CXDE$#   VFTR%  NBHGY&   MJU'(  <KIO0)  >?L+P=  _* `~    \n{ }        \b  1|47   0.2568\x1b   +3-*9";

// usを試しにいれる
//static const char PS2_KB_US[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9";
//static const char PS2_KB_SHIFT_US[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P_  _\" {+    \n} |        \b  1 47   0.2568   +3-*9";
//

#endif

#if KEY_LAYOUT == KEY_LAYOUT_JP_DVORAK
// jp-keyboard-dvorak
//static const char PS2_KB[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '1', ' ', ' ', ' ', ';', 'o', 'a', ',', '2', ' ', ' ', 'j', 'q', 'e', '.', '4', '3', ' ', ' ', ' ', 'k', 'u', 'y', 'p', '5', ' ', ' ', 'b', 'x', 'd', 'i', 'f', '6', ' ', ' ', ' ', 'm', 'h', 'g', '7', '8', ' ', ' ', 'w', 't', 'c', 'r', '0', '9', ' ', ' ', 'v', 'z', 'n', 's', 'l', '[', ' ', ' ', '`', '-', ' ', '/', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
//static const char PS2_KB_SHIFT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '!', ' ', ' ', ' ', ':', 'O', 'A', '<', '@', ' ', ' ', 'J', 'Q', 'E', '>', '$', '#', ' ', ' ', ' ', 'K', 'U', 'Y', 'P', '%', ' ', ' ', 'B', 'X', 'D', 'I', 'F', '^', ' ', ' ', ' ', 'M', 'H', 'G', '&', '*', ' ', ' ', 'W', 'T', 'C', 'R', ')', '(', ' ', ' ', 'V', 'Z', 'N', 'S', 'L', '{', ' ', ' ', '~', '_', ' ', '?', '}', ' ', ' ', ' ', ' ', '\n', '+', ' ', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
static const char PS2_KB[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '1', ' ', ' ', ' ', ';', 'o', 'a', ',', '2', ' ', ' ', 'j', 'q', 'e', '.', '4', '3', ' ', ' ', ' ', 'k', 'u', 'y', 'p', '5', ' ', ' ', 'b', 'x', 'd', 'i', 'f', '6', ' ', ' ', ' ', 'm', 'h', 'g', '7', '8', ' ', ' ', 'w', 't', 'c', 'r', '0', '9', ' ', ' ', 'v', 'z', 'n', 's', 'l', '[', ' ', ' ', '`', '-', ' ', '/', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9' };
static const char PS2_KB_SHIFT[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', '!', ' ', ' ', ' ', ':', 'O', 'A', '<', '@', ' ', ' ', 'J', 'Q', 'E', '>', '$', '#', ' ', ' ', ' ', 'K', 'U', 'Y', 'P', '%', ' ', ' ', 'B', 'X', 'D', 'I', 'F', '^', ' ', ' ', ' ', 'M', 'H', 'G', '&', '*', ' ', ' ', 'W', 'T', 'C', 'R', ')', '(', ' ', ' ', 'V', 'Z', 'N', 'S', 'L', '{', ' ', ' ', '~', '_', ' ', '?', '}', ' ', ' ', ' ', ' ', '\n', '+', ' ', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '1', '|', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9' };
#endif

#if KEY_LAYOUT == KEY_LAYOUT_US
// us-keyboard
//static const char PS2_KB[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9  ";
//static const char PS2_KB_SHIFT[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P~  _\" {+    \n} |        \b  1 47   0.2568   +3-*9  ";

// 後ろ-2
//static const char PS2_KB[] =       "`             `      q1   zsaw2  cxde43   vftr5  nbhgy6   mju78  ,kio09  ./l;p-  \\' [=    \n] \\        \b  1 47   0.2568   +3-*9";
//static const char PS2_KB_SHIFT[] = "~             ~      Q!   ZSAW@  CXDE$#   VFTR%  NBHGY^   MJU&*  <KIO)(  >?L:P_  _\" {+    \n} |        \b  1 47   0.2568   +3-*9";

// 1.3.2b13
#define PS2_KB (PS2_KB_MAP_US + 2)
#define PS2_KB_SHIFT (PS2_KB_MAP_US + 2 + 0x7f)

#endif

#if KEY_LAYOUT == KEY_LAYOUT_AZERTY
static const char PS2_KB[] =       "\0             \0      a&   wsqz\0  cxde'\"   vftr(  nbhgy-   ,ju\0_  ;kio\0\0  :!lmp)  *\0 ^=    \n$ *        \b  1 47   0.2568   +3-*9";
static const char PS2_KB_SHIFT[] = "\0             \0      Q1   ZSAW2  CXDE43   VFTR5  NBHGY6   ?JU78  .KIO09  /\0LMP\0  \0% \0+    \n\0 \0        \b  1 47   0.2568   +3-*9";
#endif


#define IME_KEYMAP 1
#define IME_JP_KANA 2
#define IME_VN_TELEX 3
#define IME_NONE 4

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

// French Azerty
#if LANG == LANG_FR
#define IME IME_NONE
static const char PS2_KB_ALT[] =       "                              ~  cxde{#   vftr[  nbhgy|   mju`\\   kio@^    l p]      }    \n           \b  1 47   0.2568   +3-*9";
#endif

// Chinese
#if LANG == LANG_ZH
#define IME IME_NONE
#endif

// Bopomofo for Taiwan
#if LANG == LANG_BP
#define IME IME_KEYMAP
static const char PS2_KB_ALT[]       = "                     \xba\xb9   \xbc\xbf\xbb\xbe\xbd  \xc3\xc0\xc2\xc1`\xde   \xc6\xc5\xc8\xc4\xc7  \xcd\xca\xcc\xc9\xcb'   \xdd\xdc\xdb\xdf\xce  \xd1\xd0\xcf\xd3\xd6\xd2  \xd5\xd9\xd4\xd8\xb9\xda  \\: @^    \n[ ]        \b  1\\47   0.2568\x1b  +3-*9";
//static const char PS2_KB_ALT[]       = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\'', '\xb9', ' ', ' ', ' ', '\xd8', '\xd3', '\xbb', '\xd1', '\xbd', ' ', ' ', '\xdc', '\xba', '\xc1', '\xd5', '`', '\xde', ' ', ' ', ' ', '\xd0', '\xdb', '\xcb', '\xb9', '\xc7', ' ', ' ', '\xca', '\xbc', '\xc2', '\xcf', '\xc5', '\'', ' ', ' ', ' ', '\xdd', '\xcc', '\xc9', '\xdf', '\xce', ' ', ' ', '\xbe', '\xc8', '\xc3', '\xc4', '\xd6', '\xd2', ' ', ' ', '\xc6', '\xbc', '\xcd', '\xbf', '\xd4', '[', ' ', ' ', '`', '\xda', ' ', '\xd9', ']', ' ', ' ', ' ', ' ', '\n', '=', ' ', '\\', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\b', ' ', ' ', '\xb9', '\\', '4', '7', ' ', ' ', ' ', '0', '.', '2', '5', '6', '8', ' ', ' ', ' ', '+', '3', '-', '*', '9', ' ', ' ' };
static const char* PS2_KB_SHIFT_ALT = PS2_KB_ALT;
#endif
//10 for i=#a0 to #df:?hex$(i);chr$(i);" ";:next

static inline uint key_getKeyboardID() {
	//return PS2_KB[-2];
	return *(uint8_t*)(0x5700);
}


#include "../romajikana.h" // 1.3b2
//uint8 key_kana_buf[2]; // xtsu/ltsu 対応する場合増やす必要あり
//char key_kana_nbuf = 0;

//#define PS2_TICK systick
#define PS2_TICK() screen_tick()
#define PS2_TICK_DIF 1

//int ps2usjp = 0; // 両対応で300byte増える



static const char* KEY_FUNC = // size変化なし
	"\x13\x0c\0" // page up, clear after cursor 1.2b13 // F1
	"\x18LOAD\0" // F2
	"\x18SAVE\0" // F3
	"\x18\x0cLIST\n\0" // \nを外した -> 戻した 0x12 -> 先頭へ、カーソル以降削除 v1.2b42 // F4
	"\x18RUN\n\0"  // F5
	"\x18?FREE()\n\0" // F6
	"\x18OUT0\n\0" // 0.9.4 // F7
	"\x18VIDEO1\n\0" // 0.9.6 // F8
	"\x18\014FILES\0" // 1.0.0b14 // \014(8進数) = 0x0c // 1.2b35 \nなくした // F9
	"\x18SWITCH\n\0" // 1.2b32 // F10
;

enum {
	KEY_F1 = 0,
	KEY_F2 = KEY_F1 + 3,
	KEY_F3 = KEY_F2 + 6,
	KEY_F4 = KEY_F3 + 6,
	KEY_F5 = KEY_F4 + 8,
	KEY_F6 = KEY_F5 + 6,
	KEY_F7 = KEY_F6 + 10,
	KEY_F8 = KEY_F7 + 7,
	KEY_F9 = KEY_F8 + 9,
	KEY_F10 = KEY_F9 + 8
};

void WAKEUP0_7_IRQHandler(void) {
	LPC_SYSCON->STARTRSRP0CLR = 1 << PS2KB_CLK; // clear irq
	
	int tick = PS2_TICK();
	if (tick - key_bkt > PS2_TICK_DIF) {
		key_data = key_nbit = key_cnt = 0;
	}
	key_bkt = tick;
	
//	key_debug++;
	if (!key_flg.enable)
		return;
	
	if (PS2_DATA()) {
		if (key_nbit > 0 && key_nbit < 9) {
			key_data |= 1 << (key_nbit - 1);
		}
		key_cnt++;
	}
	key_nbit++;
	
	if (key_nbit < 11)
		return;
//	xprintf("%2x ", key_data);
	
//	const char* PS2_KB = ps2 ? PS2_KB_JP : PS2_KB_US;
//	const char* PS2_KB_SHIFT = ps2 ? PS2_KB_SHIFT_JP : PS2_KB_SHIFT_US;

	if (!(key_cnt & 1)) {
		uint8 key = key_data;
		if (key == 0xe1) { // pause (e1, 14, 77, e1, f0, 14, f0, 77) -> F12(=0x07)相当
			key = 0x07;
		}
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
						//						keykbhit &= ~(1 << 2);
						key_kbhit_rel(2);
					} else if (key == 0x72) { // down
//						keykbhit &= ~(1 << 3);
						key_kbhit_rel(3);
					} else if (key == 0x6b) { // left
//						keykbhit &= ~(1 << 0);
						key_kbhit_rel(0);
					} else if (key == 0x74) { // right
//						keykbhit &= ~(1 << 1);
						key_kbhit_rel(1);
					} else if (key == 0x11) { // right ALT
						key_flg.alt_r = 0;
					} else if (key == 0x14) { // right CTRL
						key_flg.ctrl_r = 0;
//					} else if (key == 0x1f) { // left WIN == left ALT
//						key_flg.alt_l = 0;
					}
					key_flg.e0 = 0;
				} else {
					if (key == 0x12) { // left SHIFT
						key_flg.shift_l = 0;
					} else if (key == 0x59) { // right SHIFT
						key_flg.shift_r = 0;
					} else if (key == 0x11 || key == 0x67) { // left ALT and  == 無変換 1.2.4b50
						key_flg.alt_l = 0;
					} else if (key == 0x14) { // left CTRL
						key_flg.ctrl_l = 0;
					} else if (key == 0x29) { // space
//						keykbhit &= ~(1 << 4);
						key_kbhit_rel(4);
					//} else if (key == 0x5A) { // enter
					} else if (key == 0x22) { // X
					//} else if (key == 0x35) { // Y
						key_kbhit_rel(5);
					/* // kbhitにA-K・・・入らない
					} else {
						k = PS2_KB[key];
						if (k >= 'a' && k <= 'k') {
							key_kbhit_rel(k - ('a' - 5));
						}
						*/
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
							//							keykbhit |= 1 << 2;
							key_kbhit(2);
						} else if (key == 0x72) { // down
							k = 31;
//							keykbhit |= 1 << 3;
							key_kbhit(3);
						} else if (key == 0x6b) { // left
							k = 28;
//							keykbhit |= 1 << 0;
							key_kbhit(0);
						} else if (key == 0x74) { // right
							k = 29;
//							keykbhit |= 1 << 1;
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
						} else if (key == 0x70) { // insert key
							//key_flg.insert = !key_flg.insert;
							k = 0x11;
						} else if (key == 0x11) { // right ALT = kana -> same as left ALT -> 1.2b52 kana廃止
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
								//k = 0x11;
							/* // right ALT = kana 廃止 1.2b52
							} else {
								key_flg.kana = !key_flg.kana;
								key_kana_nbuf = 0;
							*/
							}
						} else if (key == 0x14) { // right CTRL
							key_flg.ctrl_r = 1;
//						} else if (key == 0x1f) { // left WIN == left ALT ver1.2.4b50 x
//							key_flg.alt_l = 1;
						}
						key_flg.e0 = 0;
					}
				} else {
					if (key == 0x12) { // left SHIFT
						key_flg.shift_l = 1;
						#if IME != IME_NONE
						if (key_flg.ctrl_r || key_flg.ctrl_l) { // lang
						/*
							key_flg.kana = !key_flg.kana;
							key_kana_buf0 = 0;
							*/
							k = 0xf;
						}
						#endif
					} else if (key == 0x59) { // right SHIFT
						key_flg.shift_r = 1;
						#if IME != IME_NONE
						if (key_flg.ctrl_r || key_flg.ctrl_l) { // lang
						/*
							key_flg.kana = !key_flg.kana;
							key_kana_buf0 = 0;
							*/
							k = 0xf;
						}
						#endif
					} else if (key == 0x58) { // caps
						if (key_flg.kana) {
							key_flg.kana = 0; // 0.9.4
							key_kana_buf0 = 0;
						} else {
							key_flg.caps = !key_flg.caps;
//							key_send_caps(key_flg.caps); // 割り込みの関係で微妙
						}
					} else if (key == 0x13) { // kana
#if KEY_LAYOUT != KEY_LAYOUT_AZERTY
						#if IME != IME_NONE
						key_flg.kana = !key_flg.kana;
						key_kana_buf0 = 0;
						#endif
#else
						k = key_flg.shift_r || key_flg.shift_l ? '>' : '<';
#endif
					} else if (key == 0x11 || key == 0x67) { // left ALT and == 無変換 ver1.2.4b50
						key_flg.alt_l = 1;
						if (key_flg.ctrl_r || key_flg.ctrl_l) { // INS
							//	key_flg.insert = !key_flg.insert;
							k = 0x11;
						}
					} else if (key == 0x14) { // left CTRL
						key_flg.ctrl_l = 1;
					} else if (key == 0x29) { // space
						#if IME != IME_NONE
						if (key_flg.ctrl_r || key_flg.ctrl_l) { // ctrl + space -> lang 1.4b10
						/*
							key_flg.kana = !key_flg.kana;
							key_kana_buf0 = 0;
							*/
							k = 0xf;
						} else {
							k = key_flg.shift_r || key_flg.shift_l ? 0xe : 0x20; // Shift+Space=空白挿入
							key_kbhit(4);
						}
						#else
//						k = 0x20;
						k = key_flg.shift_r || key_flg.shift_l ? 0xe : 0x20; // Shift+Space=空白挿入
//						keykbhit |= 1 << 4;
						key_kbhit(4);
						#endif
					} else if (key == 0x5A) { // enter
						//k = '\n';
						k = key_flg.shift_r || key_flg.shift_l ? 0x10 : '\n'; // Shift+Enter=行分割 ver1.0.0b14
						//key_kana_nbuf = 0; // 1.2.1
						key_kana_buf0 = 0; // 1.3b2
						//key_kbhit(5); // enter対応?
					} else if (key == 0x05) { // F1
//						key_push("\030CLS\n"); // 022(8進数) = 0x12
						//key_push("\x13\x0c"); // page up, clear after cursor 1.2b13
						key_push(KEY_FUNC + KEY_F1);
					} else if (key == 0x06) { // F2
//						key_push("\x18LOAD");
						key_push(KEY_FUNC + KEY_F2);
					} else if (key == 0x04) { // F3
//						key_push("\x18SAVE");
						key_push(KEY_FUNC + KEY_F3);
					} else if (key == 0x0c) { // F4
//						key_push("\x18\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
//						key_push("\x18\030LIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除
//						key_push("\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> カーソル以降削除 ->0x0cのみに?
//						key_push("\x18\x0cLIST\n"); // \nを外した -> 戻した 0x12 -> 先頭へ、カーソル以降削除 v1.2b42
						key_push(KEY_FUNC + KEY_F4);
					} else if (key == 0x03) { // F5
//						key_push("\x18RUN\n");
						key_push(KEY_FUNC + KEY_F5);
					} else if (key == 0x0B) { // F6
//						key_push("\x18?FREE()\n");
						key_push(KEY_FUNC + KEY_F6);
					} else if (key == 0x83) { // F7
//						key_push("\x18OUT0\n"); // 0.9.4
						key_push(KEY_FUNC + KEY_F7);
					} else if (key == 0x0A) { // F8
//						key_push("\x18VIDEO1\n"); // 0.9.6
						key_push(KEY_FUNC + KEY_F8);
					} else if (key == 0x01) { // F9
//						key_push("\x18\014FILES"); // 1.0.0b14 // \014(8進数) = 0x0c // 1.2b35 \nなくした
						key_push(KEY_FUNC + KEY_F9);
					} else if (key == 0x09) { // F10
//						key_pushc(0x18); // 0.9.9
//						key_push("\x18SWITCH\n"); // 1.2b32
						key_push(KEY_FUNC + KEY_F10);
					} else if (key == 0x78) { // F11
						key_pushc(0x0c); // 0.9.9 // カーソル以下クリア
					} else if (key == 0x07) { // F12 // numlockの0xe1ついか ver1.2b42
						key_pushc(0x18); // 1.4b13 // 行削除
//						key_pushc(0x10); // 1.0.0b14 // shift+enter
						// F7-F12:  83 0A 01 09 78 07
					} else if (key == 0x76) { // ESC
						k = 0x1b;
						//key_kana_nbuf = 0; // 1.2.1
						key_kana_buf0 = 0; // 1.3b2
					} else if (key == 0x0d) { // TAB
						key_pushc('\t');
					} else if (key == 0x7e) { // Scroll Lcok 無視、小型キーボードで起動時に送られてくる
					} else if (key == 0x77) { // num lock 無視
					} else if (key == 0x14) { // 左CTRL 無視
//#if KEY_LAYOUT == KEY_LAYOUT_JP || KEY_LAYOUT == KEY_LAYOUT_JP_DVORAK // 1.3.2b11
//					} else if (key == 0x0e) { // 半角/全角 無視 // 1.3.2b15
//#endif
					} else if (key == 0x64) { // 変換 無視
					} else if (key == 0x67) { // 無変換 無視
					} else {
						int shift = key_flg.shift_r || key_flg.shift_l;
//						if (key < 0x80) {
						if (key < 0x80 - 2) { // 1.2b6
//						if (key >= 21 && key < 0x80 - 2) { // 1.2b6 x us非対応
							if (key_flg.alt_l || key_flg.alt_r) { // 1.2b52
//							if (key_flg.alt_l) { // やっぱり左だけにする 1.1b7 -> 1.2b52 両ALT一緒に
#if LANG != LANG_FR
								// shift でかえる 0-F + G-V で32キャラ x 2
								// W-Zを0-3と同様とする ver 1.2.4b50
								k = PS2_KB[key];
								//			_printf("%x %d\n", key, k);
								if (k >= '0' && k <= '9') {
									k = k - '0' + (shift ? 0x80 : 0xe0);
								} else if (k >= 'w' && k <= 'z') { // wxyz ver1.2.4b50
									k = k - 'w' + (shift ? 0x80 : 0xe0);
								} else if (k >= 'a' && k <= 'v') {
									k = k - ('a' - 10) + (shift ? 0x80 : 0xe0);
								} else if (k == '[') { // ver 1.2b5
									k = '_';
								} else if (k == ']') { // ver 1.2b5
									k = key_flg.kana ? 0xa0 : '\\';
								} else {
									k = 0;
								}
#else
								k = PS2_KB[key];
								//			_printf("%x %d\n", key, k);
								if (k >= 'w' && k <= 'z') { // wxyz ver1.2.4b50
									k = k - 'w' + (shift ? 0x80 : 0xe0);
								} else if (k >= 'a' && k <= 'v') {
									k = k - ('a' - 10) + (shift ? 0x80 : 0xe0);
								} else {
									k = PS2_KB_ALT[key];
									if (k == ' ')
										k = 0;
								}
#endif
							} else if (!key_flg.kana) {
								if (shift) {
									k = PS2_KB_SHIFT[key];
									if (!key_flg.caps) {
										if (k >= 'A' && k <= 'Z') {
											k += 'a' - 'A';
										}
									}
								} else {
									k = PS2_KB[key];
									if (!key_flg.caps) {
										if (k >= 'a' && k <= 'z') {
											k += 'A' - 'a';
										}
									}
									if (k == 'X') { // 1.4b05
										key_kbhit(5);
									}
									/*
									// x, y 入らない
									if (k == 'X' || k == 'Y') { // 1.4b05
										key_kbhit(k -('X' - 5));
									}
									*/
									/* btnにA-K・・・入らない
									if (k >= 'A' && k <= 'K') {
										key_kbhit(k - ('A' - 5));
									}
									*/
								}
							} else { // kana
#if IME == IME_KEYMAP
								if (shift) {
									k = PS2_KB_SHIFT_ALT[key];
									/*
									if (!key_flg.caps) {
										if (k >= 'A' && k <= 'Z') {
											k += 'a' - 'A';
										}
									}
									*/
								} else {
									k = PS2_KB_ALT[key];
									/*
									if (!key_flg.caps) {
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
								if (key_kana_buf0) {
									int bk = key_kana_buf0;
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
								key_kana_buf0 = k;
								
#elif IME == IME_JP_KANA
								if (shift) {
									k = PS2_KB_SHIFT[key];
								} else {
									k = PS2_KB[key];
								}
								
								/*
									if (!key_flg.caps) {
										if (k >= 'A' && k <= 'Z') {
											k += 'a' - 'A';
										}
									}
								*/
								k =	romajikana_input(k);
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
	key_data = key_nbit = key_cnt = 0;
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
		
		if (_g.uartmode_rxd) {
			// キーフラグ シリアルからはトグルする // 1.2b47 シリアル無効時はkey状態もescも効かないようにする（いつからだ？）
			if (c >= 28 && c <= 32) {
				keykbhit ^= 1 << (c - 28);
			} else if (c == 27) {
	//			_g.key_flg_esc = 1;
	//			_g.key_flg_esc = _g.uartmode_rxd != 2; // 1.1b14
				_g.key_flg_esc = (_g.uartmode_rxd & 2) == 0; // 1.2b41
			}
			// ここまで
			
			if (c == '\r' && (_g.uartmode_rxd & 4)) { // 1.2b41 CR mode
				c = '\n';
			}
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

void UARTInit(uint32_t baudrate) { // static inlineにすると警告？
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
	//	uint32_t div = (((int)(SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV) / (int)LPC_SYSCON->UARTCLKDIV) / 16) / baudrate;
	//uint32_t div = (((SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV) / LPC_SYSCON->UARTCLKDIV) / 16) / baudrate;
	uint32_t div = calcDivU(calcDivU(calcDivU(SystemCoreClock * LPC_SYSCON->SYSAHBCLKDIV, LPC_SYSCON->UARTCLKDIV), 16), baudrate); // 1.3.2b12 +4byte なぜ??
	
	LPC_UART->DLM = div >> 8; //div / 256;
	LPC_UART->DLL = div & 0xff; //div % 256;
	LPC_UART->LCR = 0x03;		/* DLAB = 0 */
	LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
	
	/* Read to clear the line status. */
//	regVal = LPC_UART->LSR; // 1.1b14 不要？
	
	/* Ensure a clean start, no data in either TX or RX FIFO. */
	// CodeRed - added parentheses around comparison in operand of &
	while ((LPC_UART->LSR & (LSR_THRE | LSR_TEMT)) != (LSR_THRE | LSR_TEMT));
	while (LPC_UART->LSR & LSR_RDR) {
//		regVal = LPC_UART->RBR;	/* Dump data from RX FIFO */ // 1.1b14 不要？
	}
	
	// Enable the UART Interrupt
	NVIC_EnableIRQ(UART_IRQn);
	NVIC_SetPriority(UART_IRQn, 0);
	LPC_UART->IER = IER_RBR | IER_RLS;	// Enable UART interrupt
}

/*
void UARTSend(char* buf, int len) {
	while (len != 0) {
		while (!(LPC_UART->LSR & LSR_THRE));
		LPC_UART->THR = *buf;
		buf++;
		len--;
	}
}
*/

//

/*
void uart_checker() {
	if (UARTCount) {
		LPC_UART->IER = IER_THRE | IER_RLS; // Disable RBR  // 0.9.8
		int cnt = UARTCount;
		if (cnt > KEY_BUF_LEN)
			cnt = KEY_BUF_LEN;
		memcpy((void*)(keybuf + 1), (void*)UARTBuffer, cnt);
//		nkeybuf = cnt;
		*keybuf = cnt;
		UARTCount = 0;
		LPC_UART->IER = IER_THRE | IER_RLS | IER_RBR; // Re-enable RBR  // 0.9.8
		
		// キーフラグ シリアルからはトグルする
		for (int i = 0; i < cnt; i++) {
			char c = keybuf[i + 1];
			if (c >= 28 && c <= 32) {
				keykbhit ^= 1 << (c - 28);
			} else if (c == 27) {
				_g.key_flg_esc = 1;
			}
		}
	}
}

void fillKeyBuffer() {
	if (key_nbuf > 0) {
//		__disable_irq(); // 0.9.8
		NVIC_DisableIRQ(PS2_WAKEUP_IRQn);
		int cnt = key_nbuf;
		if (cnt > KEY_BUF_LEN)
			cnt = KEY_BUF_LEN;
		memcpy((void*)(keybuf + 1), (void*)key_buf, cnt);
//		nkeybuf = cnt;
		*keybuf = cnt;
		key_nbuf = 0;
//		__enable_irq(); // 0.9.8
		NVIC_EnableIRQ(PS2_WAKEUP_IRQn);
	}
	uart_checker();
}
*/

int key_getKey() {
	if (!*keybuf) {
		return -1; // 1.2b19
	}
	int res = keybuf[1];
	__disable_irq(); // 1.2b51 - 割込でバッファが壊れることがあった
	(*keybuf)--;
	int kb = *keybuf;
	for (int i = 0; i < kb; i++)
		keybuf[i + 1] = keybuf[i + 2];
	__enable_irq(); // 1.2b51
	return res;
}
void key_clearKey() {
//	keykbhit = 0; // ver1.2b5 clear しないように変更
	*keybuf = 0;
}

#endif
