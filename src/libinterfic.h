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

struct free_page{
        unsigned long page_num;
        struct free_page *next;
};

#define TEXT_SIZE       1024UL
#define MAX_NUM_CHOICES 4U
#define CHOICE_SIZE     256UL
#define PAGE_NUM_SIZE   3UL
#define PAGE_SIZE       (TEXT_SIZE + MAX_NUM_CHOICES*(CHOICE_SIZE + PAGE_NUM_SIZE))
extern const unsigned long MAX_PAGE_NUMBER;

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, const unsigned long TOTAL_PAGES, const unsigned long NUM_PAD_PAGES);
extern unsigned discoverFreePages(struct free_page **const free_pages, unsigned long *const total_pages, FILE *const fp);
extern void forgetFreePages(struct free_page *free_pages);
extern unsigned insertPage(FILE *const fp, const unsigned long PAGE_NUM, const unsigned char *const PAGE_DATA, struct free_page **const free_pages);
extern unsigned writeFicHeader(FILE *fp);
extern void writePageNumber(unsigned char *fic_page_num, const unsigned long PAGE_NUM);

#endif
