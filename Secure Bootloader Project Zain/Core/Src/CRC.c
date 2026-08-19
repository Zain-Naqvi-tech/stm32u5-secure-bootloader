/*
 * CRC.c
 *
 *  Created on: Aug 18, 2026
 *      Author: zainn
 */

#include <stdint.h>

#include "stm32u585xx.h"
#include "CRC.h"

void CRC_RESET(void) {
	CRC->CR |= (1U << 0); //set bit 0 to RESET Value to INIT
}

void CRC_Init(void) {
	//enable clock
	RCC->AHB1ENR |= (1U << 12); //CRC Clock Enabled in RCC_AHB1ENR

	//Set the INIT value
	CRC->INIT = 0xFFFFFFFF;

	//RESET the data values in the CRC_CR before every data transfer
	CRC_RESET();
}

