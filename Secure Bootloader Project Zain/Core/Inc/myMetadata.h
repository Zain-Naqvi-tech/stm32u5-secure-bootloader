/*
 * myMetadata.h
 *
 *  Created on: Aug 18, 2026
 *      Author: zainn
 */

#ifndef MYMETADATA_H_
#define MYMETADATA_H_

#include "flash.h"

typedef struct {
	uint32_t magicIdentifier; //4 bytes
	uint32_t imageVersion; //4 bytes
	uint32_t checksum; //4 bytes
	char activeSlot; //1 byte

	//add 3 bytes of reserved data to complete 16 bytes (details in notes regarding not trusting padding especially for flash writes)
	uint8_t reserved[3];
} Metadata;

extern Metadata MD;

//returns the quad-word[4] needed to be written into the metadata region of FLASH
void MetaData_QuadWord(Metadata *MD, uint32_t *quad_word);
void MetaData_Verify(Descriptor *Slot);

#endif /* MYMETADATA_H_ */
