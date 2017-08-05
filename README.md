# beaglebone_starterware-android
Controlling beaglebone through android application

Youtube Video Link: https://www.youtube.com/watch?v=UvdkpXEQvR4

DESCRIPTION
Beaglebone runs the Android accessory protocol (generic) and accessory application specific software. We use the TI's Starterware for Beaglebone as the base software and have developed & integrated the android accessory protocol and a sample application. We have also developed a dummy app (apk) for Android that runs on phone or tablet that interacts with Beaglebone application to control the peripherals on Beaglebone.
Functionally, Beaglebone is connected to Android powered (phone / tablet) device over USB, the Beaglebone operates USB in host mode (powers the bus and enumerates devices) and the Android-powered device acts as the USB device. Beaglebone runs the Android USB accessory protocol with application software, detects & enumerates the Android-powered device (phone/tablet) and starts the sample application (beaglebone side) that waits for commands from Android device (running sample accessory apk) to control/monitor the Beaglebone peripherals viz on chip LED's, RTC,watchdog timer and gpio pins of the board.


BUILDING SOURCES

The procedure to download the sources, installing the toolchain and compiling the sources to generate the final images (like pre-built images) is described below

1.	Getting The Toolchain

•	Download CodeSourcery tool chain for ARM for Linux

•	Install toolchain on Linux Host machine (ubuntu 10.04 or above)

   	#> chmod 777 arm-2009q1-161-arm-none-eabi.bin

   	#> ./arm-2009q1-161-arm-none-eabi.bin

•	Set PATH environment variable contain the path of the compiler/tool chain.
 
  Example:

  	#> export PATH=$PATH:/opt/tools/CodeSourcery/Sourcery_G++_Lite/bin

•	Point LIB_PATH shell environment variable to the Code Sourcery installation

   Example:

   	LIB_PATH=/opt/tools/CodeSourcery/Sourcery_G++_Lite


Note: /opt/tools/ is the path selected while installing the toolchain. If you are installing toolchain path at some other location, please set PATH and LIB_PATH variable appropriately.

2. Getting Source

As mentioned above, we use TI's StarterWare software package to develop Beaglbone Android accessory.

•	Download StartWare Linux Installer package.

•	Install StarterWare package

   	#> <path to package dir>/AM335X_StarterWare_02_00_00_07_Setup.bin

•	Download the Beaglebone Accessory patch to StarterWare.

•	Apply the patch.

	#> cd <StarterWare 02.00.00.07>

	#> patch -p1< <patch to patch directory>/adk_patch/0001-Add-beaglebone-Accessory-support.patch

3. Building Bootloader

Use below commands to build the StarterWare bootloader, this will generate boot_ti.bin image at <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/bootloader/Release
   
   	#> cd <Path to StarterWare Code>/build/armv7a/gcc/am335x/beaglebone/bootloader

   	#> make

NOTE: Rename "boot_ti.bin" to MLO.

  	#> mv <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/bootloader/Release/boot_ti.bin <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/bootloader/Release/MLO


4. Building Android Accessory Starterware Application

Use below command to build the sample Beaglebone accessory application for StarterWare. This will generate an application usb_acc_ti.bin binary at <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/usb_acc/Release/usb_acc_ti.bin

   	#> cd <Path to StarterWare Code>/build/armv7a/gcc/am335x/beaglebone/usb_acc

   	#> make

NOTE: Rename Application "usb_acc_ti.bin" to "app".

   	#> mv <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/usb_acc/Release/usb_acc_ti.bin <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/usb_acc/Release/app
   
   Note: Replace usb_acc folder files in Starterware source code

   
5. Preparing the bootable SD/MMC card

Copy compiled images and mk-mmc-acc.sh script to image folder and populate SD/MMC card as follows.


   	#> mkdir ~/image

   	#> cd ~/image

   	#> cp <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/bootloader/Release/MLO .

   	#> cp <Path to StarterWare code>/binary/armv7a/gcc/am335x/beaglebone/usb_acc/Release/app .

   	#> cp <path to pre-built image>/BeagleBoneAcc/mkmmc-acc.sh .

   	#> sudo ./mkmmc-acc.sh /dev/sd<device>


Above steps will create a bootable SD/MMC card which should be similar to pre-built images and can be used to bootup beaglebone.

6. Android App

Android Eclipse project can be found in BBonedroid folder. Build it and run it.