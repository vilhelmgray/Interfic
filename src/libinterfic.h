/* Copyright (C) 2014 William Breathitt Gray
 *
 * This file is part of Interfic.
 *
 * Interfic is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Interfic is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Interfic.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef LIBINTERFIC_H
#define LIBINTERFIC_H

#include <stdio.h>
#include <stdint.h>

#include "free_pages.h"

#define HEADER_SIZE     9UL

#define TEXT_SIZE       1024UL
#define MAX_NUM_CHOICES 4U
#define CHOICE_SIZE     256UL
#define PAGE_NUM_SIZE   3UL
#define PAGE_SIZE       (TEXT_SIZE + MAX_NUM_CHOICES*(CHOICE_SIZE + PAGE_NUM_SIZE))
extern const unsigned long MAX_PAGE_NUMBER;

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, unsigned long *const total_pages, const unsigned long NUM_PAD_PAGES);
extern unsigned insertPage(FILE *const fp, const unsigned long PAGE_NUM, const uint8_t *const PAGE_DATA, struct free_page **const free_pages, unsigned long *const total_pages);
extern unsigned lookupPage(FILE *const fp, const unsigned long PAGE_NUM, uint8_t *const page_data);
extern unsigned writeFicHeader(FILE *fp);
extern void writePageNumber(uint8_t *fic_page_num, const unsigned long PAGE_NUM);

#endif
