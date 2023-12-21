/*!
    \file  usbh_hid_keybd.c 
    \brief this file is the application layer for usb host hid keyboard handling
           qwerty and azerty keyboard are supported as per the selection in 
           usbh_hid_keybd.h

    \version 2019-6-5, V1.0.0, firmware for GD32VF103
*/

/*
    Copyright (c) 2019, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "usbh_hid_keybd.h"
#include <stdio.h>

#define PS_LS 0x0012   // Left-Shift
#define PS_RS 0x0059   // Right-Shift
#define PS_LC 0x0014   // Left-Ctrl
#define PS_RC 0xE014   // Right-Ctrl
#define PS_LA 0x0011   // Left-Alt
#define PS_RA 0xE011   // Right-Alt

static const uint16_t g_usbps2[] = 
{
    0x0000,    0x0000,    0x0000,    0x0000,    0x001C,    0x0032,    0x0021,    0x0023, 
    0x0024,    0x002B,    0x0034,    0x0033,    0x0043,    0x003B,    0x0042,    0x004B,          /* 0x00 - 0x0F */
    0x003A,    0x0031,    0x0044,    0x004D,    0x0015,    0x002D,    0x001B,    0x002C,
    0x003C,    0x002A,    0x001D,    0x0022,    0x0035,    0x001A,    0x0016,    0x001E,          /* 0x10 - 0x1F */
    0x0026,    0x0025,    0x002E,    0x0036,    0x003D,    0x003E,    0x0046,    0x0045, 
    0x005A,    0x0076,    0x0066,    0x000D,    0x0029,    0x004E,    0x0055,    0x0054,          /* 0x20 - 0x2F */
    0x005B,    0x005D,    0x005D,    0x004C,    0x0052,    0x000E,    0x0041,    0x0049,  
    0x004A,    0x0058,    0x0005,    0x0006,    0x0004,    0x000C,    0x0003,    0x000B,          /* 0x30 - 0x3F */
    0x0083,    0x000A,    0x0001,    0x0009,    0x0078,    0x0007,    0x0000,    0x007E,  
    0x0000,    0xE070,    0xE06C,    0xE07D,    0xE071,    0xE069,    0xE07A,    0xE074,          /* 0x40 - 0x4F */
    0xE06B,    0xE072,    0xE075,    0x0077,    0xE04A,    0x007C,    0x007B,    0x0079,
    0xE05A,    0x0069,    0x0072,    0x007A,    0x006B,    0x0073,    0x0074,    0x006C,          /* 0x50 - 0x5F */
    0x0075,    0x007D,    0x0070,    0x0071,    0x0000,    0x0000,    0x0000,    0x0000, 
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0x60 - 0x6F */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0x70 - 0x7F */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0051,
  //0x0013,    0x0000,    0x0064,    0x0067,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0x80 - 0x8F */
    0x0013,    0x006A,    0x0064,    0x0067,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0x80 - 0x8F */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0x90 - 0x9F */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0xA0 - 0xAF */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000, 
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0xB0 - 0xBF */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000, 
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0xC0 - 0xCF */
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000, 
    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,    0x0000,          /* 0xD0 - 0xDF */
     PS_LC,     PS_LS,     PS_LA,    0x0000,     PS_RC,     PS_RS,     PS_RA,    0x0000           /* 0xE0 - 0xE7 */
};

uint8_t  g_usbkey = 0;
uint16_t g_usbkey_delay = 0;

extern struct{
    uint8_t  bit;
    uint16_t data;
}g_frames[];

extern struct{
    uint8_t r;
    uint8_t w;
    uint8_t waiting_for_r;
}g_rw ;

#define RW_INC(rw) (rw = (rw+1)&0b11111)
#define WFR_NO   0
#define WFR_FULL 1
#define WFR_ERR  2

static uint8_t g_mkey_last = 0;
static uint8_t g_keys_last[KBR_MAX_NBR_PRESSED] = {};
// static uint8_t g_nbr_keys_last = 0;



static void  keybrd_init    (void);
static void  keybrd_decode  (uint8_t *pbuf);

hid_proc HID_KEYBRD_cb= 
{
    keybrd_init,
    keybrd_decode
};



void keybrd_putsc_on(uint8_t uid){
    uint16_t sc = g_usbps2[uid];
    if(!sc) return;

    if(sc & 0xff00){
        if(g_rw.waiting_for_r) return;
        g_frames[g_rw.w].data = (((sc >> 8) & 0xff) << 1);
        RW_INC(g_rw.w);
        g_rw.waiting_for_r = (g_rw.w == g_rw.r) ? WFR_FULL : WFR_NO;
    }

    if(g_rw.waiting_for_r) return;
    g_frames[g_rw.w].data = (((sc >> 0) & 0xff) << 1);
    RW_INC(g_rw.w);
    g_rw.waiting_for_r = (g_rw.w == g_rw.r) ? WFR_FULL : WFR_NO;
}

static void keybrd_putsc_off(uint8_t uid){
    uint16_t sc = g_usbps2[uid];
    if(!sc) return;

    if(sc & 0xff00){
        if(g_rw.waiting_for_r) return;
        g_frames[g_rw.w].data = (((sc >> 8) & 0xff) << 1);
        RW_INC(g_rw.w);
        g_rw.waiting_for_r = (g_rw.w == g_rw.r) ? WFR_FULL : WFR_NO;
    }

    if(g_rw.waiting_for_r) return;
    g_frames[g_rw.w].data = (0xF0 << 1);    // release key scancode
    RW_INC(g_rw.w);
    g_rw.waiting_for_r = (g_rw.w == g_rw.r) ? WFR_FULL : WFR_NO;

    if(g_rw.waiting_for_r) return;
    g_frames[g_rw.w].data = (((sc >> 0) & 0xff) << 1);
    RW_INC(g_rw.w);
    g_rw.waiting_for_r = (g_rw.w == g_rw.r) ? WFR_FULL : WFR_NO;
}

/*!
    \brief      initialize the keyboard function.
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void  keybrd_init (void)
{
    /* call user init*/
    // usr_keybrd_init();
}

/*!
    \brief      decode the pressed keys.
    \param[in]  pbuf: pointer to the HID IN report data buffer
    \param[out] none
    \retval     none
*/
static void keybrd_decode(uint8_t *pbuf)
{
    if(pbuf[0] != g_mkey_last){
        uint8_t diff = pbuf[0] ^ g_mkey_last;
        uint8_t i = 0;
        while(i < 8){
            if((diff >> i) & 0b1){
                if((pbuf[0] >> i) & 0b1){
                    keybrd_putsc_on(0xe0 + i);      // modkey: 0xe0 ~ 0xe7
                }else{
                    keybrd_putsc_off(0xe0 + i);     // modkey: 0xe0 ~ 0xe7
                }
            }
            ++i;
        }
        g_mkey_last = pbuf[0];
    }



    uint8_t ix;
    uint8_t jx;

    /* check for the value of pressed key */
    uint8_t error = FALSE;
    for(ix = 2; ix < 2 + KBR_MAX_NBR_PRESSED; ix++){
        if ((pbuf[ix] == 0x01) || (pbuf[ix] == 0x02) || (pbuf[ix] == 0x03)) {
            error = TRUE;
            break;
        }
    }
    if(error == TRUE) return;



    for(ix = 0; ix < KBR_MAX_NBR_PRESSED; ix++){
        if(!g_keys_last[ix]) break;

        uint8_t off = 1;
        for(jx = 2; jx < 2 + KBR_MAX_NBR_PRESSED; jx++){
            if(g_keys_last[ix] == pbuf[jx]){
                off = 0;
                break;
            }
        }

        if(off){
            keybrd_putsc_off(g_keys_last[ix]);
            if(g_keys_last[ix] == g_usbkey) g_usbkey = 0;
        }
    }



    uint8_t nbr_keys = 0;
    uint8_t nbr_keys_new = 0;
    uint8_t keys[KBR_MAX_NBR_PRESSED] = {};
    uint8_t keys_new[KBR_MAX_NBR_PRESSED] = {};

    for(ix = 2; ix < 2 + KBR_MAX_NBR_PRESSED; ix++){
        if(pbuf[ix]){
            keys[nbr_keys++] = pbuf[ix];

            uint8_t new = 1;
            for(jx = 0; jx < KBR_MAX_NBR_PRESSED; jx++){
                if(!g_keys_last[jx]) break;

                if(pbuf[ix] == g_keys_last[jx]){
                    new = 0;
                    break;
                }
            }

            if(new) keys_new[nbr_keys_new++] = pbuf[ix];
        }
    }

    if(nbr_keys_new){
        for(ix = 0; ix < nbr_keys_new; ix++){
            keybrd_putsc_on(keys_new[ix]);
        }

        g_usbkey = keys_new[nbr_keys_new - 1];
        g_usbkey_delay = 500;
    }

    for (ix = 0; ix < KBR_MAX_NBR_PRESSED; ix++) {
        g_keys_last[ix] = keys[ix];
    }
}

