/*
 * uart.c
 *
 *  Created on: Jul 28, 2026
 *      Author: zainn
 */

#include "uart.h"

#include <stdint.h>

void USART1_Init(void) {

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
	RCC->APB2ENR |= (1U << 14);  //set bit 14 of the APB2ENR Register in order to enable the clock for USART1

	//add a delay to allow the clock to be enabled
	volatile uint32_t delay;
	delay = RCC->APB2ENR;

	//baud rate (115200)
	USART1->CR1 &= ~(1U << 15); //oversampling by 16
	USART1->BRR = 1389U; //USARTDIV=1389

	//configure stop bits (1)
	USART1->CR2 &= ~(3U << 12); //clear bits 12 and 13 to set ONE stop bit for the USART

	//configure word length (8 bits) - 1 start bit, 8 Data bits, n stop bits
	USART1->CR1 &= ~(1U << 28);
	USART1->CR1 &= ~(1U << 12);

	//no parity
	USART1->CR1 &= ~(1U << 10); //clear bit 10, which means parity control disabled

	//enable TX
	USART1->CR1 |= (1U << 3); //set bit 3 for transmitter enable

	//enable RX
	USART1->CR1 |= (1U << 2); //set bit 2 for receiver enable

	//enable peripheral
	USART1->CR1 |= (1U << 0); //USART enabled

}

void USART1_WriteChar(const char character) {

	while (!(USART1->ISR & (1U << 7))) {} //while FIFO is not full, keep looping
	USART1->TDR = character; //write the char into the TDR bits [8:0] for transmission

}

void USART1_WriteString(const char *string) {

	while (*string != '\0') {
		if (*string == '\n') {
			USART1_WriteChar('\r');
		}

		USART1_WriteChar(*string);
		string++;
	}

}

