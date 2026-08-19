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

#define M_ID 0xDEADBEEF

//0xDEADBEEF shows up as 3735928559
Metadata MD = {.magicIdentifier = M_ID, .imageVersion = 5, .checksum = 999, .activeSlot = 1}; //initial definitions

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

//this function will allow us to loop through the quad-words, from the starting address of the metadata region. The loop will find us the LATEST entry into the metadata region. This will be used in main alongside return_specifics to see if the flash memory works as expected and we can change the metadata as needed
uint32_t* MetaData_Latest_Entry(Metadata *MD, Descriptor *Slot) { //changes MD on its own and returns a LOCATION for the next append

	uint8_t flag = 1; //flag used to exit the while loop once the 'latest' is found
	uint32_t *address = (uint32_t*)Slot->start; //address starts at the metadata address

	if (address[0] != M_ID) { //empty case
		return address; //nothing written yet so we can start with the first location
	}

	while (flag) {

		//let's find the checksum for the data being written in
		CRC_RESET();
		CRC->DR = address[0];
		CRC->DR = address[1];
		CRC->DR = address[3];

		uint32_t checksum_val = CRC->DR;

		if ((address[0] == M_ID) && (address[2] == checksum_val)) {
			address += 4; //increment by 16 bytes (4 uint32 positions). Example: 0x0810_0000 + 16 = 0x0810_0010
		}
		else { //if an empty one is found

			address = address - 4; //go back to the LATEST FILLED quad_word address

			//fill up the struct with these new values for LATEST quad_word entry
			MD->magicIdentifier = address[0];
			MD->imageVersion = address[1];
			MD->checksum = address[2];
			MD->activeSlot = address[3];

			flag = 0; //exit the while loop by clearing the flag
			return (address + 4); //the address available for write function

		}

	}

	//add a guaranteed return to avoid compiler error warnings
	return (uint32_t*)Slot->start; //just return the start address of the Slot (default)

}

void MetaData_Update_Fields(Metadata *MD, int act_Slot) {

	MD->activeSlot = act_Slot;

}
