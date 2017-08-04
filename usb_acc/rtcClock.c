/**
 * \file    demoTimer.c
 *
 * \brief   This file contains Timer related functions.
 *
*/

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
*/
/* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#include "soc_AM335x.h"
#include "interrupt.h"
#include "evmAM335x.h"
#include "uartStdio.h"
#include "rtc.h"
#include "demoRtc.h"

/*******************************************************************************
**                       INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define MASK_HOUR                        (0xFF000000u)
#define MASK_MIN                         (0x00FF0000u)
#define MASK_SEC                         (0x0000FF00u)
#define MASK_MERIDIEM                    (0x000000FFu)

#define SHIFT_HOUR                       (24u)
#define SHIFT_MIN                        (16u)
#define SHIFT_SEC                        (8u)

#define MASK_DAY                         (0xFF000000u)
#define MASK_MON                         (0x00FF0000u)
#define MASK_YEAR                        (0x0000FF00u)
#define MASK_DOTW                        (0x000000FFu)

#define SHIFT_DAY                        (24u)
#define SHIFT_MON                        (16u)
#define SHIFT_YEAR                       (8u)

/*******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
static void RTCIsr(void);

/*******************************************************************************
**                      INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/
unsigned int rtcSetFlag = FALSE;
unsigned int rtcSecUpdate = FALSE;

/*******************************************************************************
**                     FUNCTION DEFINITIONS
*******************************************************************************/
/*
** Registers RTC interrupts
*/
void RtcIntRegister(void)
{
    /* Register the ISR in the Interrupt Vector Table.*/
    IntRegister(SYS_INT_RTCINT, RTCIsr);
}

/*
** Enables RTC seconds interrupt
*/
void RtcSecIntEnable(void)
{
    /* Enable interrupts to be generated on every second.*/
    RTCIntTimerEnable(SOC_RTC_0_REGS, RTC_INT_EVERY_SECOND);
}

/*
** Initializes the RTC peripheral
*/
void RtcInit(void)
{
    /* Disabling Write Protection for RTC registers.*/
    RTCWriteProtectDisable(SOC_RTC_0_REGS);

    /* Selecting Internal Clock source for RTC. */
    RTC32KClkSourceSelect(SOC_RTC_0_REGS, RTC_INTERNAL_CLK_SRC_SELECT);

    /* Enabling RTC to receive the Clock inputs. */
    RTC32KClkClockControl(SOC_RTC_0_REGS, RTC_32KCLK_ENABLE);

    RTCEnable(SOC_RTC_0_REGS);
}

/*
** Sets the Time and Calender in the RTC. This is a blocking call. 
** The time and date are entered through UART.
*/
void RtcTimeCalSet(unsigned int time)
{
 
    /* Set the time registers of RTC with the received time information.*/
    RTCTimeSet(SOC_RTC_0_REGS, time);

    /* Run the RTC. The seconds tick from now on.*/
    RTCRun(SOC_RTC_0_REGS);
 
    UARTPuts("\n\rThe Time and Date are set successfully! \n\n\r", -1);

    rtcSetFlag = TRUE;
}

/*
** Displays the Time and Date on the UART console
*/
void RtcTimeCalDisplay(unsigned char *hour, unsigned char *min, unsigned char *sec)
{
    unsigned int time = 0;
    unsigned int cal = 0;
    unsigned int temp;
 
    UARTPuts("\r", -1);

    time = RTCTimeGet(SOC_RTC_0_REGS);
    UARTPuts("Current Time And Date: ", -1);
 
    temp = (time & MASK_HOUR) >> SHIFT_HOUR;
    UARTPutc(((temp >> 4) & 0x0F) + 48);
    UARTPutc((temp & 0x0F) + 48);
  
    UARTPutc(':');
 
    temp = (time & MASK_MIN) >> SHIFT_MIN;
    UARTPutc(((temp >> 4) & 0x0F) + 48);
    UARTPutc((temp & 0x0F) + 48);

    UARTPutc(':');

    temp = (time & MASK_SEC) >> SHIFT_SEC;
    UARTPutc(((temp >> 4) & 0x0F) + 48);
    UARTPutc((temp & 0x0F) + 48);

    UARTPuts(", ", -1);

    temp = (time & MASK_HOUR) >> SHIFT_HOUR;
    *hour = ((temp >> 4) & 0x0F) *10 + (temp & 0x0F) ;
    temp = (time & MASK_MIN) >> SHIFT_MIN;
    *min = ((temp >> 4) & 0x0F) *10 + (temp & 0x0F) ;
    temp = (time & MASK_SEC) >> SHIFT_SEC;
    *sec = ((temp >> 4) & 0x0F) *10 + (temp & 0x0F) ;

}

/*
** Interrupt service routine for RTC
*/
static void RTCIsr(void)
{
    rtcSecUpdate = TRUE;
}

/******************************** End of file **********************************/



