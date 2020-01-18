/*!
    \file  main.c
    \brief USB CDC ACM device

    \version 2019-6-5, V1.0.0, demo for GD32VF103
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

#include "drv_usb_hw.h"
#include "cdc_acm_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd/lcd.h"
#include "gd32v_pjt_include.h"


extern uint8_t packet_sent, packet_receive;
extern uint32_t receive_length;

int bytes_received = 0;
int bytes_sent = 0;

usb_core_driver USB_OTG_dev = 
{
    .dev = {
        .desc = {
            .dev_desc       = (uint8_t *)&device_descriptor,
            .config_desc    = (uint8_t *)&configuration_descriptor,
            .strings        = usbd_strings,
        }
    }
};


/*!
    \brief      main routine will construct a USB keyboard
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cdc_print (usb_dev *pudev, uint8_t *str, uint32_t data_len);

int main(void)
{
    int idx = 0;
    int n_str = 0;
    int str_ovf = 0;
    uint8_t buffer[LCD_W*LCD_H+1] = "";
    uint8_t rcvstr[64] = "";
    uint8_t tmpbuf[128];
    uint16_t str_color = GREEN;

    // Initialize LEDs
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1|GPIO_PIN_2);
    LEDR(1);
    LEDG(0);
    LEDB(1);

    // Initialize Display
    Lcd_Init();
    LCD_Clear(BLACK);
    BACK_COLOR=BLACK;

    LCD_ShowStr(0, 0, (u8 *)("USB CDC/ACM example"), GREEN, OPAQUE);
    LCD_ShowStr(0, 32, (u8 *)("WAIT..."), BLUE, OPAQUE);

    eclic_global_interrupt_enable();

    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL2_PRIO2);

    usb_rcu_config();

    usb_timer_init();

    usb_intr_config();

    usbd_init (&USB_OTG_dev, USB_CORE_ENUM_FS, &usbd_cdc_cb);

    /* check if USB device is enumerated successfully */
    while (USBD_CONFIGURED != USB_OTG_dev.dev.cur_status) {
    }

    LCD_ShowStr(0,  32, (u8 *)("Connect terminal"), YELLOW, OPAQUE);
    LCD_ShowStr(0,  48, (u8 *)("to test"), YELLOW, OPAQUE);
    memset(buffer, 0, sizeof(buffer));

    while (1) {
        if (USBD_CONFIGURED == USB_OTG_dev.dev.cur_status) {
            if (1 == packet_receive && 1 == packet_sent) {
                packet_sent = 0;
                /* receive data from the host when the last packet data have been sent to the host */
                //cdc_acm_data_receive(&USB_OTG_dev);
                usbd_ep_recev (&USB_OTG_dev, CDC_ACM_DATA_OUT_EP, rcvstr, 64);
            } else {
                if (0 != receive_length) {
                    bytes_received += receive_length;
                    for (int i=0; i<receive_length; i++) {
                        if ((rcvstr[i] == '\r') || (rcvstr[i] == '\n')) {
                            buffer[idx+1] = 0;
                            if (strlen((char *)buffer) > 0) {
                                tmpbuf[0] = 0;
                                if (str_ovf > 0) {
                                    sprintf((char *)tmpbuf, "[ovf=%d]", str_ovf);
                                    int len = strlen((char *)tmpbuf);
                                    memcpy(buffer+100-len, tmpbuf, len);
                                }
                                LCD_Clear(BLACK);
                                LCD_ShowStr(0, 0, buffer, str_color, OPAQUE);
                                n_str++;
                            }
                            idx = 0;
                            str_ovf = 0;
                            memset(buffer, 0, sizeof(buffer));
                        }
                        if ((rcvstr[i] >= 32) && (rcvstr[i] < 128)) {
                            // Copy received data to buffer
                            if (idx < 100) {
                                buffer[idx] = rcvstr[i];
                                idx++;
                                // echo received characters
                                cdc_print(&USB_OTG_dev, &rcvstr[i], 1);
                            }
                            else {
                                str_ovf++;
                                tmpbuf[0] = '>';
                                cdc_print(&USB_OTG_dev, tmpbuf, 1);
                            }
                        }
                        else {
                            // test for control characters
                            if (rcvstr[i] == 18) {str_color = RED; LEDR(0); LEDG(1); LEDB(1);}         // Ctrl+R -> red color
                            else if (rcvstr[i] == 2) {str_color = BLUE; LEDR(1); LEDG(1); LEDB(0);}    // Ctrl+B -> blue color
                            else if (rcvstr[i] == 7) {str_color = GREEN; LEDR(1); LEDG(0); LEDB(1);}   // Ctrl+G -> green color
                            else if (rcvstr[i] == 25) {str_color = YELLOW; LEDR(0); LEDG(0); LEDB(1);} // Ctrl+Y -> yellow color
                            else if (rcvstr[i] == 3) {str_color = CYAN; LEDR(1); LEDG(0); LEDB(0);}    // Ctrl+C -> cyan color
                            else if (rcvstr[i] == 23) {str_color = WHITE; LEDR(0); LEDG(0); LEDB(0);}  // Ctrl+W -> white color
                            else if (rcvstr[i] == 4) LCD_Clear(BACK_COLOR);                            // Ctrl+D -> clear screen
                            else if (rcvstr[i] == 12) {LCD_Clear(DGRAY); LCD_ShowLogo(27);}            // Ctrl+L -> show logo
                            else if (rcvstr[i] == 19) {
                                // Ctrl+S -> show status
                                LCD_Clear(DGRAY);
                                LCD_ShowLogo(0);
                                sprintf((char *)tmpbuf, "Received: %d", bytes_received);
                                LCD_ShowStr(0, 32, tmpbuf, YELLOW, TRANSPARENT);
                                sprintf((char *)tmpbuf, "    Sent: %d", bytes_sent);
                                LCD_ShowStr(0, 48, tmpbuf, YELLOW, TRANSPARENT);
                                sprintf((char *)tmpbuf, " Strings: %d", n_str);
                                LCD_ShowStr(0, 64, tmpbuf, YELLOW, TRANSPARENT);
                            }
                            if ((rcvstr[i] == '\r') || (rcvstr[i] == '\n')) {
                                tmpbuf[0] = '\r';
                                tmpbuf[1] = '\n';
                                tmpbuf[2] = 0;
                                cdc_print(&USB_OTG_dev, tmpbuf, 2);
                            }
                            else {
                                tmpbuf[0] = '.';
                                cdc_print(&USB_OTG_dev, tmpbuf, 1);
                            }
                        }
                    }
                    receive_length = 0;
                }
            }
        }
        else {
            LCD_Clear(DGRAY);
            LCD_ShowLogo(0);
            LCD_ShowStr(0, 48, (u8 *)("USB NOT ENUMERATED!"), RED, TRANSPARENT);
            /* wait until USB device is enumerated again */
            while (USBD_CONFIGURED != USB_OTG_dev.dev.cur_status) {
            }
        }
    }
}


// Write to USB CDC, reading should be similar
void cdc_print (usb_dev *pudev, uint8_t *str, uint32_t data_len)
{
    packet_sent = 0;
    usbd_ep_send(pudev, CDC_ACM_DATA_IN_EP, str, data_len);
    bytes_sent += data_len;
}