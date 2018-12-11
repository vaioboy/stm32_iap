#include "stm32_mini.h"

/**
  * @brief  Precise delay function.
  * @param  Delay time in us, range from 1us to 1_864_135us
  * @retval None
  */
void mini_delay_us(uint32_t us)
{
	SysTick->LOAD = us*9;
	SysTick->CTRL = ~(1<<2);
	while(!(SysTick->CTRL&(1<<16)));
	SysTick->CTRL = 0;
}

/**
  * @brief  The printf function redirects.
  * @param  ch
  * @param  f
  * @retval int
  */
int fputc(int ch, FILE* f)
{
	while(USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET);
	
	USART_SendData(DEBUG_UART, ch);
	
	return ch;
}

/**
  * @brief  Configure D4 and D5 LED to off state.
  * @param  None
  * @retval None
  */
void mini_led_config(void)
{
	GPIO_InitTypeDef gpio_struct;
	
	GPIO_RCC_CMD(LED_RCC, ENABLE);
	
	gpio_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_struct.GPIO_Speed = GPIO_Speed_2MHz;
	gpio_struct.GPIO_Pin = LED_D4 | LED_D5;
	GPIO_Init(LED_GPIO, &gpio_struct);
	
	LED_D4_OFF;
	LED_D5_OFF;
}

/**
  * @brief  Configure debug UART and enable interrupt.
  * @param  None
  * @retval None
  */
void mini_uart_config(void)
{
	// structure define
	USART_InitTypeDef usart_struct;
	GPIO_InitTypeDef gpio_struct;
	NVIC_InitTypeDef nvic_struct;
	
	// clock enable
	DEBUG_UART_RCC_CMD(DEBUG_UART_RCC, ENABLE);
	GPIO_RCC_CMD(AFIO_RCC, ENABLE);
	GPIO_RCC_CMD(DEBUG_UART_GPIO_RCC, ENABLE);
	
	// GPIO initialization
	gpio_struct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_struct.GPIO_Pin = DEBUG_UART_TX;
	GPIO_Init(DEBUG_UART_GPIO, &gpio_struct);
	
	gpio_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_struct.GPIO_Pin = DEBUG_UART_RX;
	GPIO_Init(DEBUG_UART_GPIO, &gpio_struct);
	
	// NVIC	initialization
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	nvic_struct.NVIC_IRQChannel = DEBUG_UART_IRQ;
	nvic_struct.NVIC_IRQChannelPreemptionPriority = DEBUG_UART_PRIPRIO;
	nvic_struct.NVIC_IRQChannelSubPriority = DEBUG_UART_SUBPRIO;
	nvic_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_struct);
	
	// USART initialization
	usart_struct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	usart_struct.USART_Parity = USART_Parity_No;
	usart_struct.USART_StopBits = USART_StopBits_1;
	usart_struct.USART_WordLength = USART_WordLength_8b;
	usart_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_struct.USART_BaudRate = DEBUG_UART_BAUDRATE;
	USART_Init(DEBUG_UART, &usart_struct);
	
	USART_Cmd(DEBUG_UART, ENABLE);
	
	USART_ITConfig(DEBUG_UART, USART_IT_RXNE, ENABLE);
}

/**
  * @brief  Writes the STM32 embedded Flash memory.
  * @param  addr: address to write.
  * @param  pdata: pointer to data to be written.
  * @param  len: size of bytes to write.
  * @retval None
  */
void stm32_flash_write(uint32_t addr, uint8_t *pdata, uint32_t len)
{	
	uint32_t addr_off;
	uint8_t sector_pos;
	uint16_t sector_off;
	uint16_t sector_remain;
	
	addr_off = addr - FLASH_BASE;
	sector_pos = addr_off / FLASH_PAGE_SIZE;
	sector_off = addr_off % FLASH_PAGE_SIZE;
	sector_remain = FLASH_PAGE_SIZE - sector_off;
	
	uint16_t sector_write;
	uint8_t buff[FLASH_PAGE_SIZE];
	uint32_t i;
	uint16_t data;
	
	if(addr < FLASH_BASE || addr >= FLASH_BASE + FLASH_PAGE_SIZE*FLASH_PAGE_NUM)
		return;
	
	FLASH_Unlock();
	
	do
	{
		// check if sector has already been erased
		if(len <= sector_remain)
			sector_write = len;
		else
			sector_write = sector_remain;
		
		len -= sector_write;
		
		stm32_flash_read(addr, buff, sector_write);
		
		for(i = 0; i < sector_write; i++)
		{
			if(buff[i] != 0xFF)
				break;
		}
		if(i < sector_write)
		{
			FLASH_ErasePage(FLASH_BASE + FLASH_PAGE_SIZE*sector_pos);
		}
		
		for(i = 0; i < sector_write/2; i++)
		{
			data = pdata[2*i+1] << 8 | pdata[2*i];
			FLASH_ProgramHalfWord(addr, data);
			addr += 2;			
		}
		
		pdata += sector_write;
		
		sector_remain = FLASH_PAGE_SIZE;
		sector_pos++;
	}
	while(len > 0);
	
	FLASH_Lock();
}

/**
  * @brief  Reads the STM32 embedded Flash memory.
  * @param  addr: address to write.
  * @param  pdata: pointer to data to be written.
  * @param  len: size of bytes to write.
  * @retval None
  */
void stm32_flash_read(uint32_t addr, uint8_t *pdata, uint32_t len)
{
	uint32_t i;
	
	for(i = 0; i < len; i++)
	{
		*(pdata + i) = *((uint8_t*)addr+i);
	}
}

/**
  * @brief  Configure TIM to blink LED.
  * @param  None
  * @retval None
  */
void mini_tim_config(void)
{
	TIM_TimeBaseInitTypeDef tim_timebase_struct;
	NVIC_InitTypeDef nvic_struct;
	
	LED_TIM_RCC_CMD(LED_TIM_RCC, ENABLE);
	
	nvic_struct.NVIC_IRQChannel = LED_TIM_IRQ;
	nvic_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_struct);
	
	TIM_TimeBaseStructInit(&tim_timebase_struct);
	tim_timebase_struct.TIM_CounterMode = TIM_CounterMode_Up;
	tim_timebase_struct.TIM_Prescaler = LED_TIM_PSC;
	tim_timebase_struct.TIM_Period = LED_TIM_PERIOD;
	tim_timebase_struct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(LED_TIM, &tim_timebase_struct);
	
	TIM_Cmd(LED_TIM, ENABLE);
	
	TIM_ITConfig(LED_TIM, TIM_IT_Update, ENABLE);
}
