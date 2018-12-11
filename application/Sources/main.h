#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"
#include "stm32_mini.h"
#include <stdio.h>

// bootloader size max	8KB
#define ADDR_APP		0x08002000
#define ADDR_UPDATE		0x08006000
#define SECTOR_SIZE		0x800
#define MAX_APP_SIZE	8192

void mini_config(void);

#endif
