#include "main.h"

uint8_t long_array[MAX_APP_SIZE];
uint16_t cnt;
uint8_t flag_data;

void Iap_Load_App(u32 AppAddr);

int main(void)
{
	mini_config();
	
	printf("Hello, this is bootloader\n");
	
	while (1)
	{
		if(*((uint8_t*)ADDR_UPDATE) == 0xFF)
		{
			if(*((uint8_t*)ADDR_APP) == 0xFF)
			{
				while(flag_data == 0);
				
				flag_data = 0;
				
				mini_delay_us(500000);
				
				stm32_flash_write(ADDR_APP, long_array, cnt);
				cnt = 0;
				
				Iap_Load_App(ADDR_APP);
			}
			else
			{
				Iap_Load_App(ADDR_APP);
			}
		}
		else
		{
			for(uint8_t i=0;i<8;i++)
			{
				FLASH_Unlock();
				FLASH_ErasePage(ADDR_APP + i*SECTOR_SIZE);
				FLASH_Lock();
				
				stm32_flash_read(ADDR_UPDATE+i*SECTOR_SIZE, long_array, SECTOR_SIZE);
				stm32_flash_write(ADDR_APP+i*SECTOR_SIZE, long_array, SECTOR_SIZE);
				
				FLASH_Unlock();
				FLASH_ErasePage(ADDR_UPDATE+i*SECTOR_SIZE);
				FLASH_Lock();
			}
			Iap_Load_App(ADDR_APP);
		}
	}
}

void mini_config(void)
{
	mini_led_config();
	mini_uart_config();
	mini_tim_config();
}

typedef  void (*IapFun)(void);
IapFun JumpToApp; 

void Iap_Load_App(u32 AppAddr)
{
	if(((*(vu32*)AppAddr)&0x2FFE0000)==0x20000000)
	{ 
		JumpToApp = (IapFun)(*(vu32*)(AppAddr+4));
		__set_MSP(*(__IO uint32_t*)AppAddr);
		JumpToApp();
	}
}
