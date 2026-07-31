/*
 * uart.h
 *
 *  Created on: Jul 28, 2026
 *      Author: zainn
 */

#include "stm32u585xx.h"

#ifndef INC_UART_H_
#define INC_UART_H_

void USART1_Init(void);
void USART1_WriteChar(const char character);
void USART1_WriteString(const char *string);

#endif /* INC_UART_H_ */
