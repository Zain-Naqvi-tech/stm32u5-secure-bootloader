/*
 * flash.h
 *
 *  Created on: Aug 13, 2026
 *      Author: zainn
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

#define PGAERR 5
#define PROGERR 3

typedef struct {
	uint32_t size;
	uint32_t start;
	uint32_t bank;
} Descriptor;

int unlock_flash_nscr(void);
int lock_flash_nscr(void);
int page_erase(const Descriptor *Slot);
int write_flash(uint32_t quad_word[4], uint32_t *address);
void invalidate_icache(void);

extern const Descriptor SlotA;
extern const Descriptor SlotB;
extern const Descriptor SlotMetadata;

#endif /* FLASH_H_ */
