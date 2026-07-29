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
	GPIOA->MODER &= ~(3U << (9*2));
	GPIOA->MODER |= (2U << (9*2));

	//Make Port A Pin 10 (alternate function) mode
	GPIOA->MODER &= ~(3U << (10*2));
	GPIOA->MODER |= (2U << (10*2));

	//Set the alternate function AF7 setting for Pins 9 and 10 using the AFRH register (Pins 8 to 15)
	GPIOA->AFR[1] |= 0x00000770;

	//enable the clock for USART1
	RCC->APB2ENR |= (1U << RCC_APB2ENR_USART1EN);  //set bit 14 of the APB2ENR Register in order to enable the clock for USART1

}
