/*
 * DWT.c
 *
 *  Created on: Aug 22, 2026
 *      Author: zainn
 */

#include "DWT.h"

#include "stm32u585xx.h"

void DWT_Init(void) {

	//enable trace and debug in the DEMCR register
	CoreDebug->DEMCR |= (1U << 24); //set bit 24 to enable trace and debug in the DEMCR register

	DWT->CYCCNT = 0; //resetting the counter

	//enable the CYCCNT counter bit (0)
	DWT->CTRL |= (1U << 0); //set bit 0 to enable the counter

}
