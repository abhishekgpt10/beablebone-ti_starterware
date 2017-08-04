/**
 * \file    demoLedIf.c
 *
 * \brief   This file contains LED interface related functions.
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
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"
#include "hw_control_AM335x.h"
#include "gpio_cntl.h"

/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define LED_TOGGLE                           (0x01u)
#define LED_OFF                              (GPIO_PIN_LOW)

/*******************************************************************************
**                     INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/
static unsigned char ledState = GPIO_PIN_LOW;

/*******************************************************************************
**                          FUNCTION DEFINITIONS
*******************************************************************************/


void UserGPIOPinMuxSetup(void)
{
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(5)) = CONTROL_CONF_MUXMODE(7);
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(6)) = CONTROL_CONF_MUXMODE(7);
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(7)) = CONTROL_CONF_MUXMODE(7);
    HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(8)) = CONTROL_CONF_MUXMODE(7);
}


void LedIfConfig(void)
{

    GPIO1ModuleClkConfig();
    UserGPIOPinMuxSetup();

    GPIO1PinMuxSetup(GENERIC_GPIO_6);
    GPIO1PinMuxSetup(GENERIC_GPIO_7);
    GPIO1PinMuxSetup(GENERIC_GPIO_12);
    GPIO1PinMuxSetup(GENERIC_GPIO_13);
    GPIO1PinMuxSetup(GENERIC_GPIO_14);
    GPIO1PinMuxSetup(GENERIC_GPIO_15);

    /* Enabling the GPIO module. */
    GPIOModuleEnable(SOC_GPIO_1_REGS);

    /* Resetting the GPIO module. */
    GPIOModuleReset(SOC_GPIO_1_REGS);
}

void GpioWrite()
{
	/* Setting the GPIO pin as an output pin. */
    GPIODirModeSet(SOC_GPIO_1_REGS, LED1_GPIO, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS, LED2_GPIO, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS, LED3_GPIO, GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS, LED4_GPIO, GPIO_DIR_OUTPUT);

    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_6,GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_7,GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_12,GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_13,GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_14,GPIO_DIR_OUTPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_15,GPIO_DIR_OUTPUT);
}



void GpioRead()
{
	/* Setting the GPIO pin as an input pin. */
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_6,GPIO_DIR_INPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_7,GPIO_DIR_INPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_12,GPIO_DIR_INPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_13,GPIO_DIR_INPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_14,GPIO_DIR_INPUT);
    GPIODirModeSet(SOC_GPIO_1_REGS,GENERIC_GPIO_15,GPIO_DIR_INPUT);
}

/*
** Toggle the LED state
*/
void LedToggle(int pin_no)
{
    ledState ^= LED_TOGGLE;  
    GPIOPinWrite(SOC_GPIO_1_REGS, pin_no, ledState); 
}

/*
** Turn the  LED Off.
*/
void GpioOff(int pin_no)
{
    GPIOPinWrite(SOC_GPIO_1_REGS, pin_no, GPIO_PIN_LOW);
	Delay(0x3FFFF);
}
/*
** Turn the  LED Off.
*/
void GpioOn(pin_no)
{
    GPIOPinWrite(SOC_GPIO_1_REGS, pin_no, GPIO_PIN_HIGH);
	Delay(0x3FFFF);
}

void Delay(volatile unsigned int count)
{
    while(count--);
}
/****************************** End of file **********************************/



