/*
 * PLL.c
 *
 *  Created on: Jul 28, 2026
 *      Author: zainn
 */

#include "PLL.h"

void PLL_Init(void) {

	//Set the PLL to integer mode by Clearing PLL1FRACEN in RCC_PLL1CFGR. Bit 4
	RCC->PLL1CFGR &= ~(1U << 4);

	//enable HSI Clock (Bit 8 for HSI16)
	RCC->CR |= (1U << 8);

	while (!(RCC->CR & (1U << 10))) {}

	//choose HSI16 clock as the PLL1 entry clock source. Bit 1
	RCC->PLL1CFGR &= ~(3U << 0);
	RCC->PLL1CFGR |= (1U << 1);

	//init pre-divider to be div=1 - Bits 11:8 - 0000 (PLLM)
	RCC->PLL1CFGR &= ~(0xFU << 8); //clear all 4 bits to make it 0000 for div=1

	//set PLL1 input frequency range (between 8 and 16) MHz - Bits 3:2 - 11
	RCC->PLL1CFGR &= ~(3U << 2); //clear the two bits
	RCC->PLL1CFGR |= (3U << 2); //set the two bits to make it 0b11

	//clear bits 16 and 17 for PEN and QEN Divider output enables
	RCC->PLL1CFGR &= ~(3U << 16);

	//set PLL1REN (bit 18)
	RCC->PLL1CFGR |= (1U << 18);

	//clear P from PLL1DIVR
	RCC->PLL1DIVR &= ~(0x7FU << 9);

	//clear Q from PLL1DIVR
	RCC->PLL1DIVR &= ~(0x7FU << 16);

	//set R from PLL1DIVR to make it 0000001 for the divisor to be 2
	RCC->PLL1DIVR &= ~(0x7FU << 24);
	RCC->PLL1DIVR |= (1U << 24);

	//set N from PLL1DIVR (19) (0x013) (we need N=20)
	RCC->PLL1DIVR &= ~(0x1FFU << 0);
	RCC->PLL1DIVR |= (0x013U << 0);

	//enable PLL1 by setting PLL1ON in RCC_CR register
	RCC->CR |= (1U << 24);

	//busy-wait
	while (!(RCC->CR & (1U << 25))) {}

}
