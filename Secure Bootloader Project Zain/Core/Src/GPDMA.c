/*
 * GPDMA.c
 *
 *  Created on: Aug 20, 2026
 *      Author: zainn
 */


#include "GPDMA.h"
#include "stm32u585xx.h"

void GPDMA_Direct_Programming(uint32_t address) {

	//initialize DMA channel

	//enable clock
	RCC->AHB1ENR |= (1U << 0); //set bit 0 of the RCC_AHB1ENR register to enable clock for GPDMA1

	//make it non-secure
	GPDMA1->SECCFGR &= ~(1U << 0); //clear bit 0 to make channel 0 non-secure

	//make it privileged
	GPDMA1->PRIVCFGR |= (1U << 0); //set bit 0 to make it privileged (channel 0)

	//pick the source using the source address register
	GPDMA1_Channel0->CSAR = address; //we are using the passed in parameter for the address (source)

	//pick the destination using the destination address register
	GPDMA1_Channel0->CDAR = 0x420C0404; //source address + offset

	//work on the channel x transfer register
	GPDMA1_Channel0->CTR1 &= ~(0x03U << 0);
	GPDMA1_Channel0->CTR1 |= (0x02U << 0); //set bits 0 and 1 to 10 to make it 4-byte data width for source address (32-bits)

	//work on the channel x transfer register
	GPDMA1_Channel0->CTR1 &= ~(0x03 << 16);
	GPDMA1_Channel0->CTR1 |= (0x02 << 16); //set bits 0 and 1 to 10 to make it 4-byte data width for destination address

	//enable source incrementing
	GPDMA1_Channel0->CTR1 |= (1U << 3); //set bit 3 for contiguously incremented burst

	//disable destination incrementing
	GPDMA1_Channel0->CTR1 &= ~(1U << 19); //set bit 19 for contiguously incremented burst

	//set Transfer Complete Event Mode to 00
	GPDMA1_Channel0->CTR2 &= ~(0x3U << 30); //clear bits 30 and 31 to make transfer complete mode at block level (once the block is transmitted)

	//enable destination hardware request (DREQ)
	GPDMA1_Channel0->CTR2 |= (1U << 10); //set bit 10 so that so that selected hardware request driven by a destination peripheral (more of a handshake thing)

	//clear SWREQ so that the hardware request is taken into account
	GPDMA1_Channel0->CTR2 &= ~(1U << 9); //clear bit 9

	//set the REQSEL value to 89 for hash_in_dma
	GPDMA1_Channel0->CTR2 |= (89U << 0); //set the value to 89

	//use the block register to write the number of bytes to transfer from the source
	GPDMA1_Channel0->CBR1 |= (4U << 0); //set bits 0-15 to the number 4 to indicate that we are moving 4 bytes (32 bits) of data

	//Interrupt Enable and IRQ setup

	//enable a 'transfer complete' interrupt
	GPDMA1_Channel0->CCR |= (1U << 8); //set bit 8 to enable an interrupt which signals the system that the transfer is DONE

	//enable an error interrupt
	GPDMA1_Channel0->CCR |= (1U << 10); //data transfer error interrupt enable on bit 10 being set

	//set priority of the interrupt in the NVIC
	NVIC_SetPriority(GPDMA1_Channel0_IRQn, 1); //priority 1 (does not really matter in our case but we must assign it a value)

	//Enable the interrupt request in the NVIC
	NVIC_EnableIRQ(GPDMA1_Channel0_IRQn); //enable the Channel 0 GPDMA interrupt request handler

	//enable DMA channel
	GPDMA1_Channel0->CCR |= (1U << 0); //set bit 0 of GPDMA_CxCR to enable dma channel

}
