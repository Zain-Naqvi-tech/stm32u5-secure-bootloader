/*
 * myMetadata.c
 *
 *  Created on: Aug 18, 2026
 *      Author: zainn
 */

#include <stdint.h>

#include "stm32u585xx.h"
#include "myMetadata.h"
#include "CRC.h"
#include "uart.h"

//0xDEADBEEF shows up as 3735928559
Metadata MD = {.magicIdentifier = 0xDEADBEEF, .imageVersion = 5, .checksum = 999, .activeSlot = 1}; //initial definitions

void MetaData_QuadWord(Metadata *MD, uint32_t *quad_word) {
	quad_word[0] = MD->magicIdentifier;
	quad_word[1] = MD->imageVersion;
	quad_word[3] = MD->activeSlot;

	//reset CRC and add all the required values one by one
	CRC_RESET();
	CRC->DR = quad_word[0];
	CRC->DR = quad_word[1];
	CRC->DR = quad_word[3];

	MD->checksum = CRC->DR; //store the checksum value
	quad_word[2] = MD->checksum;
}

void MetaData_Verify(Descriptor *Slot) {
	//carve out the address needed
	uint32_t *function_address = (uint32_t*)Slot->start;

	if (function_address[0] != 0xDEADBEEF) {
		USART1_WriteString("Magic Identifiers Different - FAIL\n");
		return;
	}

	//before reading to verify, let's do a checksum check for the metadata struct quad-word
	CRC_RESET();
	CRC->DR = function_address[0];
	CRC->DR = function_address[1];
	CRC->DR = function_address[3];

	uint32_t flashChecksum = CRC->DR;
	if (flashChecksum == function_address[2]) {
		USART1_WriteString("CHECKSUM SAME\n");
	}
	else {
		USART1_WriteString("CHECKSUM MISMATCH\n");
		return;
	}
}
