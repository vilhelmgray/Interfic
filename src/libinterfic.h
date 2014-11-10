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

struct fic_choice{
        char text[CHOICE_SIZE];
        unsigned long page_num;
};
struct fic_page{
        char text[TEXT_SIZE];
        struct fic_choice choice[MAX_NUM_CHOICES];
};

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, unsigned long *const total_pages, const unsigned long NUM_PAD_PAGES);
extern unsigned readPage(FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *read_page);
extern unsigned writeFicHeader(FILE *fp);
extern unsigned writePage(FILE *const fp, const unsigned long PAGE_NUM, const struct fic_page *const NEW_PAGE, struct free_page **const free_pages, unsigned long *const total_pages);

#endif
