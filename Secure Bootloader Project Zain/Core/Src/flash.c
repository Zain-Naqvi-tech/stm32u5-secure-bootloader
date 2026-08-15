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

	//Now we can write to the memory

}

void lock_flash_nscr(void) {
	//Lock the FLASH control register FLASH_NSCR
	FLASH->NSCR |= (1U << 31); //set the 31st bit in order to LOCK the flash register

	//check if it is locked using uart print
	if (FLASH->NSCR & (1U << 31)) {
		USART1_WriteString("Locked\n");
	}

}

void page_erase(void) {
	//check that no flash memory operation is ongoing by checking the BSY bit
	while ((FLASH->NSSR) & (1U << 16)) {}

	//check and clear all error programming flags due to a previous programming
	FLASH->NSSR = 0x20FB; //this register operates on a rw_cl system where writing a 1 clears the bit. So, a hex number is used which keeps the remaining bits 0 and writes 1 to the error bits which clears them
	if ((FLASH->NSSR) & (1U << 7)) { //if PGSERR is set, it means we have not cleared it properly
		while(1) {} //a sort of breakpoint if anything goes wrong
	}

	//Set PER bit and select the page to erase (PNB) with the associated bank (BKER) in FLASH_NSCR
	FLASH->NSCR |= (1U << 1); //page erase enabled (PER)
	FLASH->NSCR &= ~(0xFFU << 3); //clear the 8 bits responsible for choosing the page number (PNB)
	FLASH->NSCR |= (0x04 << 3); //set the 8 bits to 0x04 in order to write to Page 4 (first page of slot B)
	FLASH->NSCR |= (1U << 11); //Bank 2 selected for page erase (BKER)

	//set STRT in FLASH_NSCR
	FLASH->NSCR |= (1U << 16);

	//wait for BSY to be cleared
	while ((FLASH->NSSR) & (1U << 16)) {}

	FLASH->NSCR &= ~(1U << 1); //Disable page erase (PER) to return to a safe state

	//check if the erase was successful
	if (FLASH->NSSR & 0x20FB) {
		while(1) {} //erase failed
	}

	//check the actual address. If it is all 0xFF bytes then the erase was a success
	volatile uint32_t *addr = (volatile uint32_t*)0x08108000;
	volatile uint32_t fail = 0;
	volatile uint32_t success = 0;
	uint32_t COUNT = 2048;
	for (uint32_t i = 0; i < COUNT; i++) {
		if (addr[i] == 0xFFFFFFFF) {}
		else {
			fail++;
		}
	}
	if (fail == 0) {
		USART1_WriteString("ERASE SUCCESSFUL\n");
	}
	else {
		USART1_WriteString("ERASE FAIL\n");
	}
}



