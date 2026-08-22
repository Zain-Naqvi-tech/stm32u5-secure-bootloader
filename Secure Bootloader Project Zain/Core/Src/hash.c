/*
 * hash.c
 *
 *  Created on: Aug 20, 2026
 *      Author: zainn
 */


#include "hash.h"
#include "stm32u585xx.h"

void HASH_Init(uint32_t size) {
	//enable peripheral clock
	RCC->AHB2ENR1 |= (1U << 17); //sets bit 17 in order to enable the HASH peripheral clock

	//select the algorithm to be used in the HASHING (SHA2-256)
	HASH->CR &= ~(0x3U << 17); //clear bits 17 and 18
	HASH->CR |= (0x3U << 17); //set bits 17 and 18 to select the SHA2-256 algorithm

	//Data type selection. Selecting 8-bit data to ensure it swaps to little-endian to match the compiler side
	HASH->CR &= ~(0x03U << 4); //clear the 2 bits needed for datatype
	HASH->CR |= (0x02U << 4); //set the bits to b10 for 8-bit data

	//Set the NBLW bits in the HASH_STR register. It sets the number of valid bits in the last word. This is for data which is not aligned to 4 bytes
	HASH->STR &= ~(31U << 0); //clear bits 0-4
	HASH->STR |= (((size%4)*8) << 0); //set the 4 bits to the leftover BITS needed. Resolves the deficit issue

	//set INIT bit to 1
	HASH->CR |= (1U << 2); //set bit 2 to reset the hash processor core, so that the HASH is ready to compute the message digest of the new message

	//set DMAE to 1 to enable DMA
	HASH->CR |= (1U << 3); //set bit 3 to enable DMA transfer. Hardware will clear this while the last data of the message is written to the processor
}
