#include "main.h"

uint8_t long_array[MAX_APP_SIZE];
uint16_t cnt;
uint8_t flag_data;

void Iap_Load_App(u32 AppAddr);

int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, ADDR_APP-FLASH_BASE);
	
	mini_config();
	
	printf("Hello, this is MINI APP\n");
	
	while (1)
	{
		if(flag_data == 1)
		{
			flag_data = 0;
			
			mini_delay_us(500000);
			
			if(cnt < 1024)
			{
				for(uint16_t i=0;i<cnt;i++)
				{
					USART_SendData(DEBUG_UART, long_array[i]);
					while(USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET);					
				}
				cnt = 0;
			}
			else
			{
				stm32_flash_write(ADDR_UPDATE, long_array, cnt);
				cnt = 0;
			
				NVIC_SystemReset();
			}
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
