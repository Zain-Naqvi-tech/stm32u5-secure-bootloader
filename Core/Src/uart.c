/*
 * uart.c
 *
 *  Created on: Jul 28, 2026
 *      Author: zainn
 */

#include "uart.h"

void USART_Init(void) {

	//initialize Port A
	RCC->AHB2ENR1 |= 0x01; //set bit 0 of the register to enable the clock for port A

	//Make Port A Pin 9 (alternate function) mode
	GPIOA->MODER &= ~(3U << 18);
	GPIOA->MODER |= (2U << 18);

	//Make Port A Pin 10 (alternate function) mode
	GPIOA->MODER &= ~(3U << 20);
	GPIOA->MODER |= (2U << 20);

	//Set the alternate function AF7 setting for Pins 9 and 10 using the AFRH register (Pins 8 to 15)
	GPIOA->AFR[1] &= ~(0xFU << 4);
	GPIOA->AFR[1] |= (0x7U << 4);
	GPIOA->AFR[1] &= ~(0xFU << 8);
	GPIOA->AFR[1] |= (0x7U << 8);

	//set the kernel clock source to the system clock using the first two bits of the RCC peripherals independent clock configuration register 1
	RCC->CCIPR1 &= ~(3U << 0);
	RCC->CCIPR1 |= (1U << 0); //choose the SYSCLK currently running at 160MHz using PLL as the kernel clock source for USART1

	//enable the clock for USART1
	RCC->APB2ENR |= (1U << RCC_APB2ENR_USART1EN);  //set bit 14 of the APB2ENR Register in order to enable the clock for USART1



}
