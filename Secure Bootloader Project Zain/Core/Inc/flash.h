/*
 * flash.h
 *
 *  Created on: Aug 13, 2026
 *      Author: zainn
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

typedef struct {
	uint32_t size;
	uint32_t start;
	uint32_t bank;
} Descriptor;

void unlock_flash_nscr(void);
void lock_flash_nscr(void);
void page_erase(const Descriptor *Slot);
void write_flash(uint32_t quad_word[4], const Descriptor *Slot);
void invalidate_icache(void);

extern const Descriptor SlotA;
extern const Descriptor SlotB;
extern const Descriptor SlotMetadata;

#endif /* FLASH_H_ */
