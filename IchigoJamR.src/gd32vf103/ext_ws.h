#ifndef __EXT_WS_H__
#define __EXT_WS_H__

/*
[0]=10
[1]=8
[2]=0
WS.OUT 1
*/

/*
10 fori=0to11:let[i*3],rnd(10),rnd(10),rnd(10):next
20 n=rnd(11)+1:ws.led12/n+1,n+1
25 wait10
30 run

10 [m]=cos(n)/30+10
12 if rnd(300)=0 clv:m=rnD(3)
20 n=n+1
30 ws.led1,12
40 goto 10
*/

// v3
// aligned after timer16 aligned must be 16, 今はちょうどいい
//__attribute__ ((aligned(16))) __attribute__ ((section(".text.WS_OUT_ASM"))) static const char WS_OUT_ASM[] = {
__attribute__ ((aligned(2))) static const uint8_t WS_OUT_ASM[] =  {
	0x85, 0x48, 0x63, 0x45, 0xA0, 0x00, 0x33, 0x05, 0xA0, 0x40, 0x89, 0x48, 0x13, 0x58, 0x00, 0x01,
	0x05, 0x08, 0x13, 0x15, 0x05, 0x01, 0x13, 0x55, 0x05, 0x01, 0x32, 0x88, 0x3D, 0x71, 0x22, 0xC0,
	0x26, 0xC2, 0x11, 0x47, 0x32, 0x07, 0x05, 0x07, 0x22, 0x07, 0x21, 0x07, 0x22, 0x07, 0x41, 0x07,
	0x85, 0x42, 0xBA, 0x02, 0x2A, 0xC4, 0x2E, 0xC6, 0xF3, 0x77, 0x04, 0x30, 0xA1, 0x47, 0x92, 0x07,
	0x83, 0xC6, 0x05, 0x00, 0xC6, 0x95, 0x23, 0x20, 0x57, 0x00, 0xFD, 0x44, 0x4D, 0x44, 0x33, 0xF6,
	0xF6, 0x00, 0x19, 0xE2, 0x9D, 0x44, 0x7D, 0x44, 0xFD, 0x14, 0xFD, 0xFC, 0x23, 0x22, 0x57, 0x00,
	0x7D, 0x14, 0x7D, 0xFC, 0x85, 0x83, 0xE5, 0xF3, 0x7D, 0x15, 0x69, 0xF9, 0x22, 0x45, 0xB2, 0x45,
	0x7D, 0x18, 0xE3, 0x45, 0x00, 0xFD, 0xF3, 0x67, 0x04, 0x30, 0x92, 0x44, 0x02, 0x44, 0x05, 0x61,
	0x82, 0x80
};
typedef void (*FUNC_WS_OUT_ASM)(int countrepeat, const uint8* data, uint32 gpiomask);

static inline void ws_out(int port, int nled, int repeat) {
	if (nled <= 0 || nled > 34 || !repeat)
		return;

	FUNC_WS_OUT_ASM ws_out_asm = (FUNC_WS_OUT_ASM)WS_OUT_ASM;
	//ws_out_asm(-(nled * 3 + (--repeat << 16)), ram + OFFSET_RAM_VAR, 14); // v3
	//ws_out_asm(-((nled * 3) | (--repeat << 16)), ram + OFFSET_RAM_VAR, 14); // v3
	ws_out_asm(-nled * 3, ram + OFFSET_RAM_VAR, repeat); // v3 ... うまくいかないのでしかたなく
}


/*

2811 (PL9823)
0 high 200nsec-500nsec, low 1200nsec-1500nsec
1 high 1200nsec-1500nsec, low 200nsec-500nsec
reset 50,000nsec

2812b before
0 high 150nsec-550nsec, low 700nsec-1000nsec
1 high 650nsec-950nsec, low 300nsec-600nsec
reset 50,000nsec

2812b new
0 high 220nsec-380nsec, low 580nsec-1000nsec
1 high 580nsec-1000nsec, low 580nsec-1000nsec
reset 280,000nsec

IchigoJam R
0 high 220nsec, low 1000nsec(2811次第)
1 high 1000nsec(2811次第), low 580nsec(2811次第)


// v3
asm15
void ws_out(int countrepeat, byte* data, byte* outmask)  // countrepat < 0 -> skip 2, count | ((repeat - 1) << 16)
* outmask = 	0x50010004 // OUT1

'	OUT1	WS2812B R11とR5を変更でOUTnに変更可能
'	R10 - data count
'	R11 - 開始アドレス 送信byte数low,送信byte数high,G1,R1,B1,G2,R2,B2 ....
'   R12 - temp
'	R13 - data value
'	R14 - gpio address
'	R15 - bit count
'   R16 - repeat count
'   R17 - skip byte (1 or 2)
'	R5 - OUT value
'	R8 - wait count
'	R9 - wait count
    MODE RV32C

@WS2812B
	R17=1
	IF R0<R10 GOTO @SKIPCOUNT
	R10=R0-R10
	R17=2
@SKIPCOUNT
	R16=R0>>16
	R16+=1
	R10=R10<<16
	R10=R10>>16

    ' test repeat 12
    ' R16=12

    'しかたなく
    R16=R12

    SP+=-2
    PUSH R8,0
    PUSH R9,1

    ' test R11=#800
    'R8=#8
    'R8<<=8
	'R11+=R8

    'R14=#40010810
	R14=#4
    R14<<=12
    R14+=1
    R14<<=8
    R14+=8
    R14<<=8
    R14+=#10
	
	R5=1
    R5<<=14

	PUSH R10,2
    PUSH R11,3

	CPSID
@LOOP_DATA
	R15=#8
    R15<<=4
	R13=[R11+0]
	R11+=R17
@LOOP_BIT
	[R14+0]L=R5
	'0 high 220nsec, low 1000nsec(2811次第)
	'1 high 1000nsec(2811次第), low 580nsec(2811次第)
	R9=33 '3clock*33=99 1:high
    R8=19 '3clock*19=57 1:low
    R12=R13&R15
    IF R12 GOTO @SKIP
	R9=7 '3clock*7=21 0:high
	R8=33 '3clock*8=99 0:low
@SKIP
	R9+=-1 'wait R9*3 clock
	IF R9 GOTO -1
	[R14+4]L=R5 ' *offset 4byte
	R8+=-1 'wait R8*3 clock
	IF R8 GOTO -1
	R15>>=1
	IF R15 GOTO @LOOP_BIT
	
	R10+=-1 '+6clock
	IF R10 GOTO @LOOP_DATA

	POP R10,2
    POP R11,3
    R16+=-1
    IF R0<R16 GOTO @LOOP_DATA

	CPSIE
    POP R9,1
    POP R8,0
    SP+=2
    RET

*/


#endif	//__EXT_WS_H__