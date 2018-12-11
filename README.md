# stm32_iap

This is a STM32 IAP (in-applicaton programming) demonstration.
It contains a bootloader program and an application program. 
Desktop application is not used and the application program is updated using a serial terminal.

There is a STM32F103RC in my Fire MINI develop board which has 256KB embedded flash memory.
The flash memory is divided into four parts (using mapped address):
	0x0800_0000 – 0x0800_1FFF	8KB		bootloader
	0x0800_2000 – 0x0800_5FFF	16KB 	app
	0x0800_6000 – 0x0800_AFFF	16KB 	temporary store area for app
	0x0800_B000 – 0x0803_FFFF 216KB 	reserved

After power on, the bootloader is running. It first check whether the temporary store area has been programmed. 
If yes, the new application firmware will be moved to app area and temporary area will be erased, then jump to the app. 
If no, it will jump to the app if it exists or will wait for new application firmware.

To increase robustness, that’s say, non-firmware data can be transmitted to MCU using serial port is achieved 
as long as the data length is less than 1024 bytes. In this simple demo, the received data is loop back.
