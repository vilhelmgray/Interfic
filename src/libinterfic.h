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

extern const unsigned long MAX_PAGE_NUMBER;

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, const unsigned long TOTAL_PAGES, const unsigned long NUM_PAD_PAGES);
extern unsigned discoverFreePages(struct free_page **const free_pages, unsigned long *const total_pages, FILE *const fp);
extern void forgetFreePages(struct free_page *free_pages);
extern unsigned writeFicHeader(FILE *fp);

#endif
