//*****************************************************************************
//
// usb_acc.c - main application code for the accessory demo example
// 
// This is part of AM 335X StarterWare   Firmware Package. modified and resued from 
// revision 6288 of the EK-LM3S3748 Firmware Package.
//
//*****************************************************************************

#include "watchdog.h"
#include "hw_types.h"
#include "interrupt.h"
#include "soc_AM335x.h"
#include "raster.h"
#include "grlib.h"
#include "usblib.h"
#include "usbhid.h"
#include "usbhost.h"
#include "android_acc.h"
#include "delay.h"
#include "uartStdio.h"
#include "cache.h"
#include "mmu.h"
#include "cppi41dma.h"
#include "demoRtc.h"
#include "gpio_cntl.h"
#include "beaglebone.h"
#include "usb_acc.h"


//*****************************************************************************
//
// The USB controller instance 
//
//*****************************************************************************
#define USB_INSTANCE 1

//*****************************************************************************
//
// The size of the host controller's memory pool in bytes.
//
//*****************************************************************************
#define HCD_MEMORY_SIZE         128

//*****************************************************************************
//
// The memory pool to provide to the Host controller driver.
//
//*****************************************************************************
unsigned char g_pHCDPool[HCD_MEMORY_SIZE];


//*****************************************************************************
//
// Declare the USB Events driver interface.
//
//*****************************************************************************
DECLARE_EVENT_DRIVER(g_sUSBEventDriver, 0, 0, USBHCDEvents);

//*****************************************************************************
//
// The global that holds all of the host drivers in use in the application.
// In this case, only the Accessory class is loaded.
//
//*****************************************************************************
static tUSBHostClassDriver const * const g_ppHostClassDrivers[] =
{
    &g_USBACCClassDriver,
    &g_sUSBEventDriver
};

//*****************************************************************************
//
// This global holds the number of class drivers in the g_ppHostClassDrivers
// list.
//
//*****************************************************************************
#define INITIAL_COUNT_VALUE          (0xFFFE0000u)
#define RELOAD_COUNT_VALUE           (0xFFFE0000u)

void WatchdogTimerSetUp(void);  //declaration wd setup
void watchdoginit(); // initializing watchdog timer
void watchdog_disable();  //disabling the watchdog  


static const unsigned int g_ulNumHostClassDrivers =
    sizeof(g_ppHostClassDrivers) / sizeof(tUSBHostClassDriver *);


tUSBHTimeOut *USBHTimeOut = NULL;

#define START_ADDR_DDR                     (0x80000000)
#define START_ADDR_DEV                     (0x44000000)
#define START_ADDR_OCMC                    (0x40300000)
#define NUM_SECTIONS_DDR                   (512)
#define NUM_SECTIONS_DEV                   (960)
#define NUM_SECTIONS_OCMC                  (1)


/* page tables start must be aligned in 16K boundary */
#ifdef __TMS470__
#pragma DATA_ALIGN(pageTable, 16384);
static volatile unsigned int pageTable[4*1024];
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=16384
static volatile unsigned int pageTable[4*1024];
#else
static volatile unsigned int pageTable[4*1024] __attribute__((aligned(16*1024)));
#endif

static void MMUConfigAndEnable(void);

//*****************************************************************************
//
// The global value used to store the mouse instance value.
//
//*****************************************************************************
static unsigned int g_ulACCInstance;


//*****************************************************************************
//
// This enumerated type is used to hold the states of the mouse.
//
//*****************************************************************************
enum
{
    //
    // No device is present.
    //
    STATE_NO_DEVICE,

    //
    // Android Accessory powered device has been detected and needs to be initialized in the main
    // loop.
    //
    STATE_ACC_INIT,

    //
    // Android Accessory powered device is connected and waiting for events.
    //
    STATE_ACC_CONNECTED,

    //
    // An unsupported device has been attached.
    //
    STATE_UNKNOWN_DEVICE,

    //
    // A power fault has occured.
    //
    STATE_POWER_FAULT
}
eUSBState;

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned int ulLine)
{
}
#endif


//*****************************************************************************
//
// This function updates the status area of the screen.  It uses the current
// state of the application to print the status bar.
//
//*****************************************************************************
void UpdateStatus(void)
{
	if(eUSBState == STATE_NO_DEVICE)
	{
		UARTprintf("No Device\n");
	}
	else if(eUSBState == STATE_ACC_CONNECTED)
	{
	        UARTprintf("Connected\n");
	}
	else if(eUSBState == STATE_UNKNOWN_DEVICE)
	{
	        UARTprintf("Unknown Device\n");
	}
	else if(eUSBState == STATE_POWER_FAULT)
	{
		UARTprintf("Power Fault\n");
	}
}

static void USBAINTCConfigure(int usbInstance)
{   

	if(usbInstance)
	{
	    /* Registering the Interrupt Service Routine(ISR). */
	    IntRegister(SYS_INT_USB1, USB1HostIntHandler);

	    /* Setting the priority for the system interrupt in AINTC. */
	    IntPrioritySet(SYS_INT_USB1, 0, AINTC_HOSTINT_ROUTE_IRQ);

	    /* Enabling the system interrupt in AINTC. */
	    IntSystemEnable(SYS_INT_USB1);
	}
	else
	{
	    /* Registering the Interrupt Service Routine(ISR). */
	    IntRegister(SYS_INT_USB0, USB0HostIntHandler);

	    /* Setting the priority for the system interrupt in AINTC. */
	    IntPrioritySet(SYS_INT_USB0, 0, AINTC_HOSTINT_ROUTE_IRQ);

	    /* Enabling the system interrupt in AINTC. */
	    IntSystemEnable(SYS_INT_USB0);
	}
}

static void USBInterruptEnable(int usbInstance)
{
    /* Enabling IRQ in CPSR of ARM processor. */
    IntMasterIRQEnable();

	/* Initializing the ARM Interrupt Controller. */
	IntAINTCInit();

    /* Configuring AINTC to receive UART0 interrupts. */
    USBAINTCConfigure(usbInstance);
}

//*****************************************************************************
//
// This is the generic callback from host stack.
//
// \param pvData is actually a pointer to a tEventInfo structure.
//
// This function will be called to inform the application when a USB event has
// occured that is outside those releated to the mouse device.  At this
// point this is used to detect unsupported devices being inserted and removed.
// It is also used to inform the application when a power fault has occured.
// This function is required when the g_USBGenericEventDriver is included in
// the host controller driver array that is passed in to the
// USBHCDRegisterDrivers() function.
//
// \return None.
//
//*****************************************************************************
void
USBHCDEvents(void *pvData)
{
    tEventInfo *pEventInfo;

    //
    // Cast this pointer to its actual type.
    //
    pEventInfo = (tEventInfo *)pvData;

    UARTprintf("USBHCDEvents : %d \n", pEventInfo->ulEvent);
    switch(pEventInfo->ulEvent)
    {
        //
        // New mouse detected.
        //
        case USB_EVENT_CONNECTED:
        {
            //
            // An unknown device was detected.
            //
            eUSBState = STATE_UNKNOWN_DEVICE;

           	UpdateStatus();
            break;
        }

        //
        // Keyboard has been unplugged.
        //
        case USB_EVENT_DISCONNECTED:
        {
            //
            // Unknown device has been removed.
            //
		UARTprintf("DISCONNECTED : %d \n", pEventInfo->ulEvent);
            eUSBState = STATE_NO_DEVICE;

            UpdateStatus();
            break;
        }
		
        default:
        {
            break;
        }
    }
}

static void cmdcontrol(unsigned char val)
{
	int gpio_no;
	switch ((val & 0xf0) )	
	{
		case LED1:
		UARTprintf("LED1 ");
		gpio_no = LED1_GPIO;
		break;
		case LED2:

		UARTprintf("LED2 ");
		gpio_no = LED2_GPIO;
		break;

		case LED3:
		gpio_no = LED3_GPIO;
		UARTprintf("LED3 ");

		break;

		case LED4:
		gpio_no = LED4_GPIO;

		UARTprintf("LED4 ");
		break;

		case WD:			// case watchdog
		if (val & 0x0f)
		{
			watchdoginit();
			UARTprintf("Watchdog Running\n");
		}
		else
		{
			UARTprintf("Watchdog Off\n");
			watchdog_disable();
		}
		break;

		case GPIO1_2:
		gpio_no = GENERIC_GPIO_2;
		UARTprintf("GPIO1_2 ");
		break;
		case GPIO1_3:
		gpio_no = GENERIC_GPIO_3;
		UARTprintf("GPIO1_3 ");
		break;		
		case GPIO1_6:
		gpio_no = GENERIC_GPIO_6;
		UARTprintf("GPIO1_6 ");
		break;
		case GPIO1_7:
		gpio_no = GENERIC_GPIO_7;
		UARTprintf("GPIO1_7 ");
		break;
		case GPIO1_12:
		gpio_no = GENERIC_GPIO_12;
		UARTprintf("GPIO1_12 ");
		break;
		case GPIO1_13:
		gpio_no = GENERIC_GPIO_13;
		UARTprintf("GPIO1_13 ");
		break;
		case GPIO1_14:
		gpio_no = GENERIC_GPIO_14;
		UARTprintf("GPIO1_14 ");
		break;
		case GPIO1_15:
		gpio_no = GENERIC_GPIO_15;
		UARTprintf("GPIO1_15 ");
		break;

		
		default:
		UARTprintf("INVALID GPIO ");
		break	;
		

	}
	if (val & 0x0f)
	{
		UARTprintf("ON \n");
		GpioOn(gpio_no);	
	}
	else
	{
		UARTprintf("OFF \n");
		GpioOff(gpio_no);
	}
}

//*****************************************************************************
//
// This is the callback from the USB HID mouse handler.
//
// \param pvCBData is ignored by this function.
// \param ulEvent is one of the valid events for a mouse device.
// \param ulMsgParam is defined by the event that occurs.
// \param pvMsgData is a pointer to data that is defined by the event that
// occurs.
//
// This function will be called to inform the application when a mouse has
// been plugged in or removed and anytime mouse movement or button pressed
// is detected.
//
// \return This function will return 0.
//
//*****************************************************************************
unsigned int
ACCCallback(unsigned int ulEvent)
{
    unsigned char buffer[2];
    unsigned int ulBytes;

	UARTprintf("Got event : %d \n", ulEvent);
    switch(ulEvent)
    {
        case USB_EVENT_CONNECTED:
        {      

            eUSBState = STATE_ACC_INIT;

            break;
        }

        case USB_EVENT_DISCONNECTED:
        {

            eUSBState = STATE_NO_DEVICE;

	    cmdcontrol(LED1 | LED_OFF);
	    cmdcontrol(LED2 | LED_OFF);
	    cmdcontrol(LED3 | LED_OFF);
	    cmdcontrol(LED4 | LED_OFF);
	    cmdcontrol(WD | WD_OFF);

	    cmdcontrol(GPIO1_2 | GPIO_OFF);
	    cmdcontrol(GPIO1_3 | GPIO_OFF);
	    cmdcontrol(GPIO1_6 | GPIO_OFF);
	    cmdcontrol(GPIO1_7 | GPIO_OFF);
	    cmdcontrol(GPIO1_12 | GPIO_OFF);
	    cmdcontrol(GPIO1_13 | GPIO_OFF);
	    cmdcontrol(GPIO1_14 | GPIO_OFF);
	    cmdcontrol(GPIO1_15 | GPIO_OFF);

            break;
        }

	case UNKNOWN_DEVICE_EVENT:
	{
            eUSBState = STATE_UNKNOWN_DEVICE;
            break;

	}

    }

    UpdateStatus();

    return(0);
}
/*
** Function to setup MMU. This function Maps three regions (1. DDR
** 2. OCMC and 3. Device memory) and enables MMU.
*/
void MMUConfigAndEnable(void)
{
    /*
    ** Define DDR memory region of AM335x. DDR can be configured as Normal
    ** memory with R/W access in user/privileged modes. The cache attributes
    ** specified here are,
    ** Inner - Write through, No Write Allocate
    ** Outer - Write Back, Write Allocate
    */
    REGION regionDdr = {
                        MMU_PGTYPE_SECTION, START_ADDR_DDR, NUM_SECTIONS_DDR,
                        MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                     MMU_CACHE_WB_WA),
                        MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW,
                        (unsigned int*)pageTable
                       };
    /*
    ** Define OCMC RAM region of AM335x. Same Attributes of DDR region given.
    */
    REGION regionOcmc = {
                         MMU_PGTYPE_SECTION, START_ADDR_OCMC, NUM_SECTIONS_OCMC,
                         MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                      MMU_CACHE_WB_WA),
                         MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW,
                         (unsigned int*)pageTable
                        };

    /*
    ** Define Device Memory Region. The region between OCMC and DDR is
    ** configured as device memory, with R/W access in user/privileged modes.
    ** Also, the region is marked 'Execute Never'.
    */
    REGION regionDev = {
                        MMU_PGTYPE_SECTION, START_ADDR_DEV, NUM_SECTIONS_DEV,
                        MMU_MEMTYPE_DEVICE_SHAREABLE,
                        MMU_REGION_NON_SECURE,
                        MMU_AP_PRV_RW_USR_RW  | MMU_SECTION_EXEC_NEVER,
                        (unsigned int*)pageTable
                       };

    /* Initialize the page table and MMU */
    MMUInit((unsigned int*)pageTable);

    /* Map the defined regions */
    MMUMemRegionMap(&regionDdr);
    MMUMemRegionMap(&regionOcmc);
    MMUMemRegionMap(&regionDev);

    /* Now Safe to enable MMU */
    MMUEnable((unsigned int*)pageTable);
}


//*****************************************************************************
//
// This is the main loop that runs the application.
//
//*****************************************************************************
volatile unsigned int value;


int main(void)
{
    unsigned int pin_read=0;
    unsigned int triggerValue = 0;  //wd trigger value
    tRectangle sRect;
    unsigned int i;
    unsigned char *src, *dest;
    unsigned char buffer[5];
    unsigned int ulBytes;
    unsigned char time[3];
    unsigned int t1 = 0;

      MMUConfigAndEnable();

	//
	//configures arm interrupt controller to generate raster interrupt 
	//
	USBInterruptEnable(USB_INSTANCE);

	//
	//Delay timer setup
	//	
	DelayTimerSetup();
	

	//
    // Register the host class drivers.
    //
    USBHCDRegisterDrivers(USB_INSTANCE, g_ppHostClassDrivers, g_ulNumHostClassDrivers);

    //
    // Update the status on the screen.
    //
    UpdateStatus();


    //
    // Open an instance of the accessory driver.  The accessory device does not need
    // to be present at this time, this just saves a place for it and allows
    // the applications to be notified when a accessory is present.
    //
    g_ulACCInstance =
        USBACCOpen(USB_INSTANCE, ACCCallback);

    //
    // Initialize the power configuration. This sets the power enable signal
    // to be active high and does not enable the power fault.
    //
    USBHCDPowerConfigInit(USB_INSTANCE, USBHCD_VBUS_AUTO_HIGH);

    //
    // Initialize the host controller stack.
    //
    USBHCDInit(USB_INSTANCE, g_pHCDPool, HCD_MEMORY_SIZE);
    USBHCDTimeOutHook(USB_INSTANCE, &USBHTimeOut);
    USBHTimeOut->Value.slNonEP0= 1;


    RtcIntRegister();
    RtcInit();

    /* Enable system interrupts */
    IntSystemEnable(SYS_INT_RTCINT);
    IntPrioritySet(SYS_INT_RTCINT, 0, AINTC_HOSTINT_ROUTE_IRQ);

    RtcSecIntEnable();

    LedIfConfig();
    GpioWrite();

    // Call the main loop for the Host controller driver.
    //
    USBHCDMain(USB_INSTANCE, g_ulACCInstance);

    //
    // The main loop for the application.
    //
    while(1)
    {
	
        switch(eUSBState)
        {
            //
            // This state is entered when the mouse is first detected.
            //
            case STATE_ACC_INIT:
            {
                eUSBState = STATE_ACC_CONNECTED;

		time[0] = 0;
		time[1] = 0;
		time[2] = 0;

		t1 = time[0] << 24 | time[1] << 16| time [2] << 8;
		RtcTimeCalSet(t1);
		 						//gpio initialization

		cmdcontrol(LED1 | LED_OFF);
		cmdcontrol(LED2 | LED_OFF);
		cmdcontrol(LED3 | LED_OFF);
		cmdcontrol(LED4 | LED_OFF);
		cmdcontrol(WD | WD_OFF);

	        cmdcontrol(GPIO1_6 | GPIO_OFF);
	        cmdcontrol(GPIO1_7 | GPIO_OFF);
	        cmdcontrol(GPIO1_12 | GPIO_OFF);
	        cmdcontrol(GPIO1_13 | GPIO_OFF);
	        cmdcontrol(GPIO1_14 | GPIO_OFF);
	        cmdcontrol(GPIO1_15 | GPIO_OFF);
	        cmdcontrol(GPIO1_2 | GPIO_OFF);
	        cmdcontrol(GPIO1_3 | GPIO_OFF);
		
                //
                // Update the status on the screen.
                //
                UpdateStatus();

                break;
            }
            case STATE_ACC_CONNECTED:
            {
		
		    ulBytes = AccessoryRead(g_ulACCInstance,buffer, 5 );
		    if (ulBytes)
		    {
			    UARTprintf("ACC READ: BUFFER : 0x%x 0x%x 0x%x 0x%x 0x%x : RET : %d \n",buffer[0], buffer[1], buffer[2], buffer[3],buffer[4], ulBytes);
		    
			switch (buffer [0])
			{
				case CMD_LED_CONTROL:
				cmdcontrol(buffer[1]);
				triggerValue += 1;  //wd				
				WatchdogTimerTriggerSet(SOC_WDT_1_REGS, triggerValue);//wd
				
				break;

				case CMD_RTC_CONTROL:
				
				break;

				case CMD_WD_CONTROL:
				cmdcontrol(buffer[1]);
				UARTprintf("Watchdog triggered\n");
				break;


				case CMD_GPIO_CONTROL_OUTPUT:
				//GPIOModuleReset(SOC_GPIO_1_REGS);
				//GpioWrite();
				cmdcontrol(buffer[1]);
				UARTprintf("GPIO triggered\n");
				break;
				
				case CMD_GPIO_CONTROL_INPUT:
				GPIOModuleReset(SOC_GPIO_1_REGS);
				GpioRead();
				pin_read = (int) buffer[1];
				pin_read = pin_read>>4;
				if(pin_read==8)                                        //for gpio1 pin 2 read
				{
					value=GPIOPinRead(SOC_GPIO_1_REGS,GENERIC_GPIO_2);
        				UARTprintf("Value at pin no. %d = %d\n",GENERIC_GPIO_2,value);
				}
				else if(pin_read==9)                                  //for gpio1 pin 3 read
				{
					value=GPIOPinRead(SOC_GPIO_1_REGS,GENERIC_GPIO_3);
        				UARTprintf("Value at pin no. %d = %d\n",GENERIC_GPIO_3,value);
				}
				else
				{
					value=GPIOPinRead(SOC_GPIO_1_REGS,pin_read);
        				UARTprintf("Value at pin no. %d = %d\n",pin_read,value);
				}
				GPIOModuleReset(SOC_GPIO_1_REGS);
				GpioWrite();
				break;
				

			default:
				UARTprintf("Invalid command\n");
			}
		    }


		    delay(100);

		/*
		 ** Check if RTC Time is set
		 */
		
		

		if(TRUE == rtcSetFlag)
		{
			if(TRUE == rtcSecUpdate)
			{ 
				rtcSecUpdate = FALSE;
				buffer[0] = CMD_RTC_CONTROL; 
				buffer[4]= value;                        //reading from pin
				RtcTimeCalDisplay(&buffer[1], &buffer[2], &buffer[3]);
				AccessoryWrite(g_ulACCInstance,buffer,5 );
			}
		}
 
                break;
            }
            case STATE_NO_DEVICE:
            {
                //
                // The Android accessory powered device is not connected so nothing needs to be done here.
                //
                break;
            }
            default:
            {
                break;
            }
        }

        //
        // Periodically call the main loop for the Host controller driver.
        //
        USBHCDMain(USB_INSTANCE, g_ulACCInstance);
    }
}


void WatchdogTimerSetUp(void)
{
    /* Configure and enable the pre-scaler clock */
    WatchdogTimerPreScalerClkEnable(SOC_WDT_1_REGS, WDT_PRESCALER_CLK_DIV_128);

    /* Set the count value into the counter register */
    WatchdogTimerCounterSet(SOC_WDT_1_REGS, INITIAL_COUNT_VALUE);

    /* Set the reload value into the load register */
    WatchdogTimerReloadSet(SOC_WDT_1_REGS, RELOAD_COUNT_VALUE);
}

void watchdoginit()
{

   /* Enable the WDT clocks */
    WatchdogTimer1ModuleClkConfig();

    /* Reset the Watchdog Timer */
    WatchdogTimerReset(SOC_WDT_1_REGS);

    /* Disable the Watchdog timer */
    WatchdogTimerDisable(SOC_WDT_1_REGS);
                                               
    /* Perform the initial settings for the Watchdog Timer */
    WatchdogTimerSetUp();

    /* Send the message to UART console */
    UARTprintf("Program Reset!\n");
    UARTprintf("Input any key at least once in every 4 seconds to avoid a further reset.\n\r");

    /* Enable the Watchdog Timer */
    WatchdogTimerEnable(SOC_WDT_1_REGS);

}

void watchdog_disable()
{
    WatchdogTimerDisable(SOC_WDT_1_REGS);
}
