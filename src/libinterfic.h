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

#define TEXT_SIZE       1024
#define MAX_NUM_CHOICES 4
#define CHOICE_SIZE     128
#define PAGE_NUM_SIZE   3
#define PAGE_SIZE       (TEXT_SIZE + MAX_NUM_CHOICES*(CHOICE_SIZE + PAGE_NUM_SIZE))
extern const unsigned long MAX_PAGE_NUMBER;

struct fic_choice{
        uint8_t text[CHOICE_SIZE];
        unsigned long page_num;
};
struct fic_page{
        unsigned long num;
        uint8_t text[TEXT_SIZE];
        struct fic_choice choice[MAX_NUM_CHOICES];
};

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, unsigned long *const total_pages, const unsigned long NUM_PAD_PAGES);
extern unsigned erasePage(FILE *const fp, const unsigned long PAGE_NUM, struct free_page **const free_pages);
extern unsigned loadFicFile(FILE *const fp, unsigned long *const total_pages, struct free_page **const free_pages);
extern unsigned readPage(FILE *const fp, struct fic_page *read_page);
extern unsigned writeFicHeader(FILE *fp);
extern unsigned writePage(FILE *const fp, const struct fic_page *const NEW_PAGE, struct free_page **const free_pages, unsigned long *const total_pages);
extern unsigned verifyFicHeader(FILE *const fp);

#endif
