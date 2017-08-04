#ifndef __USB_ACC_H__
#define __USB_ACC_H__

#define LED_ON 		0x1
#define LED_OFF		0x0
#define WD_ON 		0x1
#define WD_OFF		0x0

#define GPIO_ON 	0x0
#define GPIO_OFF	0x1

#define LED1   		0x10
#define LED2   		0x20
#define LED3   		0x30
#define LED4   		0x40
#define WD     		0x50

#define GPIO1_6 	0x60
#define GPIO1_7	 	0x70
#define GPIO1_2 	0x80
#define GPIO1_3	 	0x90
#define GPIO1_13 	0xd0
#define GPIO1_12 	0xc0
#define GPIO1_15 	0xf0
#define GPIO1_14 	0xe0


#define CMD_LED_CONTROL   		0x01
#define CMD_RTC_CONTROL   		0x02
#define CMD_WD_CONTROL   		0x03
#define CMD_GPIO_CONTROL_OUTPUT   	0x04
#define CMD_GPIO_CONTROL_INPUT 		0x05

#endif
