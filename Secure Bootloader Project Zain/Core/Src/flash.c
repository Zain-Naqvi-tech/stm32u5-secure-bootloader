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
#include "CRC.h"

const Descriptor SlotA = {.size = 992, .start = 0x08008000, .bank = 1};
const Descriptor SlotB = {.size = 992, .start = 0x08108000, .bank = 2};
const Descriptor SlotMetadata = {.size = 32, .start = 0x08100000, .bank = 2};

void invalidate_icache(void) {
	//invalidate the ICACHE BEFORE reading in order to ensure we do not read a cached value
	ICACHE->CR |= (1U << 1); //set bit 1 (CACHEINV) to invalidate entire cache
	while (!(ICACHE->SR & (1U << 1))) {} //wait until the BSYENDF bit is 1
}

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

void page_erase(const Descriptor *Slot) {
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

	if (Slot->bank == 1) {
		uint8_t PNB = (Slot->start - 0x08000000) / 0x2000;
		FLASH->NSCR |= (PNB << 3); //set the 8 bits to PNB in order to write to Page

		FLASH->NSCR &= ~(1U << 11); //clear bit 0 to select bank 1 for page erase (BKER)
	}
	if (Slot->bank == 2) {
		uint8_t PNB = (Slot->start - 0x08100000) / 0x2000;
		FLASH->NSCR |= (PNB << 3); //set the 8 bits to PNB in order to write to Page

		FLASH->NSCR |= (1U << 11); //set bit 1 to select bank 2 for page erase (BKER)
	}

	//set STRT in FLASH_NSCR
	FLASH->NSCR |= (1U << 16);

	//wait for BSY to be cleared
	while ((FLASH->NSSR) & (1U << 16)) {}

	FLASH->NSCR &= ~(1U << 1); //Disable page erase (PER) to return to a safe state

	//check if the erase was successful
	if (FLASH->NSSR & 0x20FB) {
		USART1_WriteString("ERASE FAIL (ERROR RAISED)\n");
	}

	//invalidate the ICACHE before reading
	invalidate_icache();

	//check the actual address. If it is all 0xFF bytes then the erase was a success
	volatile uint32_t *addr = (volatile uint32_t*)Slot->start;
	volatile uint32_t fail = 0;
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

void write_flash(uint32_t quad_word[4], uint32_t *address) {

	//set EOPIE bit in FLASH_NSCR to enable interrupt
	FLASH->NSCR |= (1U << 24);

	//check that no main flash memory sequence is currently ongoing
	while ((FLASH->NSSR) & (1U << 16)) {} //wait for the BSY bit to clear in order to proceed

	//check that the write buffer is empty by checking WDW
	while ((FLASH->NSSR) & (1U << 17)) {} //wait for the WDW bit to clear (technically only checking if it is clear and not really 'waiting')

	//check and clear all error programming flags due to a previous programming
	FLASH->NSSR = 0x20FB; //this register operates on a rw_cl system where writing a 1 clears the bit. So, a hex number is used which keeps the remaining bits 0 and writes 1 to the error bits which clears them
	if ((FLASH->NSSR) & (1U << 7)) { //if PGSERR is set, it means we have not cleared it properly
		while(1) {} //a sort of breakpoint if anything goes wrong
	}

	//Set PG
	FLASH->NSCR |= (1U << 0); //set bit 0 to enable FLASH programming

	//carve out the address of the page's start
	uint32_t *function_address = (uint32_t*)address;

	//write the quad_word into that address
	function_address[0] = quad_word[0];
	function_address[1] = quad_word[1];
	function_address[2] = quad_word[2];
	function_address[3] = quad_word[3];

	//check that the write buffer is empty by checking WDW
	while ((FLASH->NSSR) & (1U << 17)) {} //wait for the WDW bit to clear (technically only checking if it is clear and not really 'waiting')

	//now wait for the BSY bit to be cleared
	while ((FLASH->NSSR) & (1U << 16)) {}

	//if the EOP bit is set, the operation was a success. Proceed to clear it
	if ((FLASH->NSSR) & (1U << 0)) {
		FLASH->NSSR = 0x01; //clears EOP
	}

	//clear PG
	FLASH->NSCR &= ~(1U << 0); //clear bit 0 to disable FLASH programming

	if (FLASH->NSSR & 0x20FB) {
		USART1_WriteString("WRITE FAIL (ERROR RAISED)\n");
		if (FLASH->NSSR & (1U << 5)) {
			USART1_WriteString("PGAERR raised\n");
		}
		if (FLASH->NSSR & (1U << 3)) {
			USART1_WriteString("PROGERR raised\n");
		}
	}

	//invalidate ICACHE
	invalidate_icache();

	char string[10];

	//verify if the write was a success
	for (volatile uint32_t i = 0; i < 4; i++) {
		int_to_str(function_address[i], string);
		USART1_WriteString(string);
		USART1_WriteString("\n");
	}
	USART1_WriteString("\n");

	//do a pass/fail verification
	int fail = 0;
	for (volatile uint32_t j = 0; j < 4; j++) {
		if (function_address[j] == quad_word[j]) {}
		else {
			fail++;
		}
	}
	if (fail == 0) {
		USART1_WriteString("WRITE SUCCESSFUL\n");
	}
	else {
		USART1_WriteString("WRITE FAIL\n");
	}

}
