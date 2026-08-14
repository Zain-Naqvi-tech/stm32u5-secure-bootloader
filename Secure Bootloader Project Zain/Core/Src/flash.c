/*
 * flash.c
 *
 *  Created on: Aug 13, 2026
 *      Author: zainn
 */


#include <stdint.h>
#include <stdio.h>
#include "stm32u585xx.h"
#include "flash.h"
#include "uart.h"

void unlock_flash_nscr(void) {

	if (FLASH->NSCR & (1U << 31)) {
		USART1_WriteString("Locked\n");
	}

	//wait for the BSY bit in FLASH_NSSR register to clear in order to start writing to it
	while ((FLASH->NSSR) & (1U << 16)) {}

	//unlock FLASH control register FLASH_NSCR
	FLASH->NSKEYR = 0x45670123;
	FLASH->NSKEYR = 0xCDEF89AB;

	//if unlocked, print unlocked
	if (((FLASH->NSCR) & (1U << 31)) == 0) { //lock bit is clear
		USART1_WriteString("Unlocked\n");
	}

	//Write to the memory

}

void lock_flash_nscr(void) {
	//Lock the FLASH control register FLASH_NSCR
	FLASH->NSCR |= (1U << 31); //set the 31st bit in order to LOCK the flash register

	//check if it is locked using uart print
	if (FLASH->NSCR & (1U << 31)) {
		USART1_WriteString("Locked\n");
	}

}
