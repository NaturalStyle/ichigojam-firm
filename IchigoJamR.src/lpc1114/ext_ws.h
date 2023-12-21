#ifndef __EXT_WS_H__
#define __EXT_WS_H__

#if VER_PLATFORM != PLATFORM_LPC1114
static inline void ws_out(int port, int nled, int repeat) {
}
#else

// 288 -> 152, +136byte, +172byte

/*
[0]=10
[1]=8
[2]=0
WS.OUT 1
*/

/*
// v1
//__attribute__ ((section(".ext.ws"))) 
__attribute__ ((aligned(16))) static const uint8_t WS_OUT_ASM[] =  {
	0x00, 0x00, 0x00, 0x00, // for timing fix // 1.3.2b20
	0xF0, 0xB5, 0x9C, 0x46, 0x72, 0xB6, 0x80, 0x25, 0x0C, 0x78, 0x61, 0x44, 0xFF, 0x23, 0x03, 0x60,
	0x03, 0x27, 0x0A, 0x26, 0x2C, 0x42, 0x01, 0xD0, 0x0A, 0x27, 0x03, 0x26, 0x01, 0x3F, 0xFD, 0xD1,
	0x00, 0x23, 0x03, 0x60, 0x01, 0x3E, 0xFD, 0xD1, 0x6D, 0x08, 0xEF, 0xD1, 0x01, 0x3A, 0xEA, 0xD1,
	0x62, 0xB6, 0xF0, 0xBD
};
typedef void (*FUNC_WS_OUT_ASM)(uint32_t gpiomask, const uint8_t* data, int count, int skip);
*/

// aligned must be 16 // 78byte
//__attribute__ ((aligned(16))) static const char WS_OUT_ASM[] = {
/*
// v2/
// aligned after timer16 aligned must be 16, 今はちょうどいい
__attribute__ ((aligned(16))) __attribute__ ((section(".text.WS_OUT_ASM"))) static const char WS_OUT_ASM[] = {
 	0x00, 0x00, 0x00, 0x00, 0x01, 0xB4, 0x01, 0x20, 0x00, 0x2A, 0x01, 0xD5, 0x52, 0x42, 0x02, 0x20,
	0x84, 0x46, 0x01, 0xBC, 0xF0, 0xB5, 0x72, 0xB6, 0x0E, 0xB4, 0x80, 0x25, 0x0C, 0x78, 0x61, 0x44,
	0xFF, 0x23, 0x03, 0x60, 0x03, 0x27, 0x0A, 0x26, 0x2C, 0x42, 0x01, 0xD0, 0x0A, 0x27, 0x03, 0x26,
	0x01, 0x3F, 0xFD, 0xD1, 0x00, 0x23, 0x03, 0x60, 0x01, 0x3E, 0xFD, 0xD1, 0x6D, 0x08, 0xEF, 0xD1,
	0x01, 0x3A, 0xEA, 0xD1, 0x0E, 0xBC, 0x01, 0x3B, 0xE6, 0xD1, 0x62, 0xB6, 0xF0, 0xBD
};
typedef void (*FUNC_WS_OUT_ASM)(uint32 gpiomask, const uint8* data, int count, int repeat);
*/

// v3
// aligned after timer16 aligned must be 16, 今はちょうどいい
__attribute__ ((aligned(16))) __attribute__ ((section(".text.WS_OUT_ASM"))) static const char WS_OUT_ASM[] = {

	0x01, 0x23, 0x00, 0x28, 0x01, 0xD5, 0x40, 0x42, 0x02, 0x23, 0x9C, 0x46, 0x03, 0x0C, 0x01, 0x33,
	0x00, 0x04, 0x00, 0x0C, 0xF0, 0xB5, 0x72, 0xB6, 0x0B, 0xB4, 0x80, 0x25, 0x0C, 0x78, 0x61, 0x44,
	0xFF, 0x23, 0x13, 0x60, 0x03, 0x27, 0x0A, 0x26, 0x2C, 0x42, 0x01, 0xD0, 0x0A, 0x27, 0x03, 0x26,
	0x01, 0x3F, 0xFD, 0xD1, 0x00, 0x23, 0x13, 0x60, 0x01, 0x3E, 0xFD, 0xD1, 0x6D, 0x08, 0xEF, 0xD1,
	0x01, 0x38, 0xEA, 0xD1, 0x0B, 0xBC, 0x01, 0x3B, 0xE6, 0xD1, 0x62, 0xB6, 0xF0, 0xBD
/* // v3.1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x00, 0x28, 0x01, 0xD5, 0x40, 0x42,
	0x02, 0x23, 0x9C, 0x46, 0x03, 0x0C, 0x01, 0x33, 0x00, 0x04, 0x00, 0x0C, 0x00, 0x2A, 0x00, 0xD1,
	0x0E, 0x4A, 0xF0, 0xB5, 0x72, 0xB6, 0x0B, 0xB4, 0x80, 0x25, 0x0C, 0x78, 0x61, 0x44, 0xFF, 0x23,
	0x13, 0x60, 0x03, 0x27, 0x0A, 0x26, 0x2C, 0x42, 0x01, 0xD0, 0x0A, 0x27, 0x03, 0x26, 0x01, 0x3F,
	0xFD, 0xD1, 0x00, 0x23, 0x13, 0x60, 0x01, 0x3E, 0xFD, 0xD1, 0x6D, 0x08, 0xEF, 0xD1, 0x01, 0x38,
	0xEA, 0xD1, 0x0B, 0xBC, 0x01, 0x3B, 0xE6, 0xD1, 0x62, 0xB6, 0xF0, 0xBD, 0x04, 0x00, 0x01, 0x50,
	*/
};
typedef void (*FUNC_WS_OUT_ASM)(int countrepeat, const uint8* data, uint32 gpiomask);

//static const uint8_t data[] = { 3, 8, 2, 0, 0, 0, 1, 1, 1, 3, 0, 0, 0, 4, 0 };

/*
uint8_t data[3] = { 10, 5, 2 };
ws_out(0x50010001, data, 3, 1);
*/

/*
static inline void ws_wait_before() {
	for (volatile int i = 0; i < 500; i++);
}
*/

static inline void ws_out(int port, int nled, int repeat) {
	#if VER_PLATFORM == PLATFORM_LPC1114

	if (nled <= 0 || nled > 34 || !repeat)
		return;

/*
	GPIO1 https://fukuno.jig.jp/1476
			n	m	
		OUT	1	0	n-1
		OUT	2	1	n-1
		OUT	3	2	n-1
		OUT	4	3	n-1
		OUT	5	8	n-1+4	a=n-1, m=a+a&4
		OUT	6	9	n-1+4
		LED	7	5	n-1+4-5	a=n-1, m=a+a&4
 */
	#define GPIO1 0x50010000
	#define GPIO1_0 (GPIO1 + (1 << (0 + 2))) // OUT1 = 0x50010004
	#define GPIO1_5 (GPIO1 + (1 << (5 + 2))) // LED

//	#define WS_OUT_PORT GPIO1_0 // 1.4b06まで OUT1
	#define WS_OUT_PORT GPIO1_5 // 1.4b08 LED

	// port 1-4 と 7 が指定可能、それ以外を指定するとまずい？

/*
	port++;
	if (port == 8)
		port--;
 */
	/*
	port--;
	if (port == 6)
		port--;
	else
		port += port & 4;
	*/
//	uint WS_OUT_PORT = GPIO1 + (1 << port);
	
//	LPC_IOCON->R_PIO1_0 = 0x000000d1; // OK ... なぜ？ // 1.4b06まで
//	LPC_IOCON->R_PIO1_0 = 0x000000c1; // NG

	// なぜいる？？？ i/o まわりチェック ... なくても動くようになった？？
	/*
	LPC_IOCON->R_PIO1_0 = 0x000000c1; // ok
	LPC_GPIO1->DIR |= 1 << 0;
	*(uint32_t*)GPIO1_0 = 0;
	*/
	//

//	IJB_wait(1, 1); // なぜか wait がいる -> 16byte境界問題だった
	//for (volatile uint32_t i = 500000; i > 0; i--);
//	ws_wait_before(); // なぜかいるwait

	FUNC_WS_OUT_ASM ws_out_asm = (FUNC_WS_OUT_ASM)(WS_OUT_ASM + 1);
	//uint8_t data[] = { 3, 8, 2, 0, 0, 0, 1, 1, 1, 3, 0, 0, 0, 4, 0 };
	//ws_out_asm(GPIO1_0, data, nled * 3, 2); // ok

/*
	int16_t* var = (int16_t*)(ram + OFFSET_RAM_VAR);
	IJB_led(var[0] == 1); // ok
	ws_out_asm(GPIO1_0, var, nled * 3, 2);
*/
//	ws_out_asm(-(nled * 3 + (--repeat << 16)), ram + OFFSET_RAM_VAR, 0); // v3.1
	ws_out_asm(-(nled * 3 + (--repeat << 16)), ram + OFFSET_RAM_VAR, WS_OUT_PORT); // v3
//	ws_out_asm(WS_OUT_PORT, ram + OFFSET_RAM_VAR, -nled * 3, repeat); // v2
//	uint8_t data2[] = { 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0 };
//	ws_out_asm(GPIO1_0, data2, 6, 2); // ok

//	int (*f)(int, uint8_t*, int, int) = (void*)(WS_OUT_ASM + 1);
//	f(GPIO0_1, ram + OFFSET_RAM_VAR, nled * 3, 2);
//	int (*f)() = (void*)(WS_OUT_ASM + 1);
/*
	char d[2] = { 0x70, 0x47 };
	int (*f)() = (void*)(d);
	f();
*/
	#endif
}


/*

// v3
asm15
void ws_out(int countrepeat, byte* data, byte* outmask)  // countrepat < 0 -> skip 2, count | ((repeat - 1) << 16)
* outmask = 	0x50010004 // OUT1

'	OUT1	WS2812B DOUT [R1+`0001]を変更でOUTnに変更可能
'	R0 - data count
'	R1 - data address G1,0,R1,0,B1,0,G2,0,R2,0,B2,0 ....
'	R2 - GPIO address
'	R3 - repeat count, OUT value 0/#FF
'	R4 - data value
'	R5 - bit count
'	R6 - wait count buf
'	R7 - wait count
'	R12 - 1 or 2

' 78byte

@WS2812B
	R3=1
	R0-0
	IF PL GOTO @SKIPCOUNT
	R0=-R0
	R3=2
@SKIPCOUNT
	R12=R3
	R3=R0>>16
	R3+=1
	R0=R0<<16
	R0=R0>>16

	' default OUT1
'	R2-0
'	IF !0 GOTO @SKIPSETOUTP
'	R2=[@OUTP]L
'@SKIPSETOUTP

	PUSH {LR,R4,R5,R6,R7}
	CPSID
@LOOP_ALL
	PUSH {R0,R1,R3}
	
@LOOP_DATA
	R5=#80
	R4=[R1]
	R1+=R12
@LOOP_BIT
	R3=#FF
	[R2]L=R3
	R7=3 '(3*4+1)=13 <- 16.8+-7.2
	R6=10 '(10*4+1)=41 <- 43.2+-7.2
	R4&R5
	IF 0 GOTO @SKIP
	R7=10 '(10*4+1)=41 <- 43.2+-7.2
	R6=3 '(3*4+1)=13 <- 16.8+-7.2
@SKIP
	R7-=1 'wait R7*4+1 clock
	IF !0 GOTO -1
	R3=0
	[R2]L=R3
	R6-=1 'wait R6*4+1 clock
	IF !0 GOTO -1
	R5=R5>>1
	IF !0 GOTO @LOOP_BIT
	
	R0-=1 '+6clock
	IF !0 GOTO @LOOP_DATA
	
	POP {R0,R1,R3}
	R3-=1
	IF !0 GOTO @LOOP_ALL

	CPSIE
	POP {PC,R4,R5,R6,R7} 'RET
@OUTP
	DATA L #50010004

// v2
asm15
void ws_out(byte* outmask, byte* data, int count, int repeat)  // count < 0 -> skip 2
* outmask = 	0x50010004 // OUT1

'	OUT1	WS2812B DOUT [R1+`0001]を変更でOUTnに変更可能
'	R0 - GPIO address
'	R1 - data address G1,0,R1,0,B1,0,G2,0,R2,0,B2,0 ....
'	R2 - data count
'	R3 - OUT value 0/#FF
'	R4 - data value
'	R5 - bit count
'	R6 - wait count buf
'	R7 - wait count
'	R12 - 1 or 2

' 78byte

@WS2812B
	NOP
	NOP

	PUSH {R0}
	R0=1
	R2-0
	IF PL GOTO @SKIPCOUNT
	R2=-R2
	R0=2
@SKIPCOUNT
	R12=R0
	POP {R0}

	PUSH {LR,R4,R5,R6,R7}
	CPSID
@LOOP_ALL
	PUSH {R1,R2,R3}
	
@LOOP_DATA
	R5=#80
	R4=[R1]
	R1+=R12
@LOOP_BIT
	R3=#FF
	[R0]L=R3
	R7=3 '(3*4+1)=13 <- 16.8+-7.2
	R6=10 '(10*4+1)=41 <- 43.2+-7.2
	R4&R5
	IF 0 GOTO @SKIP
	R7=10 '(10*4+1)=41 <- 43.2+-7.2
	R6=3 '(3*4+1)=13 <- 16.8+-7.2
@SKIP
	R7-=1 'wait R7*4+1 clock
	IF !0 GOTO -1
	R3=0
	[R0]L=R3
	R6-=1 'wait R6*4+1 clock
	IF !0 GOTO -1
	R5=R5>>1
	IF !0 GOTO @LOOP_BIT
	
	R2-=1 '+6clock
	IF !0 GOTO @LOOP_DATA
	
	POP {R1,R2,R3}
	R3-=1
	IF !0 GOTO @LOOP_ALL

	CPSIE
	POP {PC,R4,R5,R6,R7} 'RET



asm15 // v1
'	OUT1	WS2812B DOUT [R1+`0001]を変更でOUTnに変更可能
'	R0 - GPIO address
'	R1 - data address G1,0,R1,0,B1,0,G2,0,R2,0,B2,0 ....
'	R2 - data count
'	R3 - OUT value 0/#FF
'	R4 - data value
'	R5 - bit count
'	R6 - wait count buf
'	R7 - wait count
'	R12 - 1 or 2
outmask = 	0x50010004 // OUT1

void ws_out(byte* outmask, byte* data, int count, int skip)

// 56byte
@WS2812B
	NOP ' for ROM border
	NOP

	PUSH {LR,R4,R5,R6,R7}
	R12=R3
	CPSID
	
@LOOP_DATA
	R5=#80
	R4=[R1]
	R1+=R12
@LOOP_BIT
	R3=#FF
	[R0]L=R3
	R7=3 '(3*4+1)=13 <- 16.8+-7.2
	R6=10 '(10*4+1)=41 <- 43.2+-7.2
	R4&R5
	IF 0 GOTO @SKIP
	R7=10 '(10*4+1)=41 <- 43.2+-7.2
	R6=3 '(3*4+1)=13 <- 16.8+-7.2
@SKIP
	R7-=1 'wait R7*4+1 clock
	IF !0 GOTO -1
	R3=0
	[R0]L=R3
	R6-=1 'wait R6*4+1 clock
	IF !0 GOTO -1
	R5=R5>>1
	IF !0 GOTO @LOOP_BIT
	
	R2-=1 '+6clock
	IF !0 GOTO @LOOP_DATA
	
	CPSIE
	POP {PC,R4,R5,R6,R7} 'RET
*/

#endif

#endif	//__EXT_WS_H__