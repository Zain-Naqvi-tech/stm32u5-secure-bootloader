/*
 * flash.h
 *
 *  Created on: Aug 13, 2026
 *      Author: zainn
 */

#ifndef FLASH_H_
#define FLASH_H_

void unlock_flash_nscr(void);
void lock_flash_nscr(void);
void page_erase(void);
void write_flash(uint32_t *address, uint32_t quad_word[4]);
void invalidate_icache(void);

#endif /* FLASH_H_ */
