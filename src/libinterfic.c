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
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "free_pages.h"
#include "libinterfic.h"

static const uint8_t MAGIC[8] = { 0x49, 0x4E, 0x54, 0x45, 0x52, 0x46, 0x49, 0x43 };
static const uint8_t VERSION = 0;

#define MAX_OFFSET      ((1UL<<31) - 1)
#define MAX_FIC_SIZE    (MAX_OFFSET + PAGE_SIZE - HEADER_SIZE)
const unsigned long MAX_PAGE_NUMBER = MAX_FIC_SIZE/PAGE_SIZE - 1;

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, unsigned long *const total_pages, const unsigned long NUM_PAD_PAGES){
        if(fseek(fp, HEADER_SIZE + (*total_pages)*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to end of file.\n");
                return 1;
        }

        struct free_page **free_pages_end = &free_pages;
        while(*free_pages_end){
                free_pages_end = &((*free_pages_end)->next);
        }
        struct free_page *const PAD_PAGES_BEGIN = *free_pages_end;

        for(unsigned long i = *total_pages; i < *total_pages + NUM_PAD_PAGES; i++){
                const unsigned char PAD_PAGE[PAGE_SIZE] = {'\0'};
                if(!fwrite(PAD_PAGE, sizeof(PAD_PAGE), 1, fp)){
                        fprintf(stderr, "Error writing padding pages.\n");
                        goto err_pad_page_write;
                }

                struct free_page *tmp_page = malloc(sizeof(*tmp_page));
                if(!tmp_page){
                        fprintf(stderr, "Unable to allocate memory for free pages list.\n");
                        goto err_free_page_malloc;
                }
                tmp_page->page_num = i;
                tmp_page->next = NULL;

                *free_pages_end = tmp_page;
                free_pages_end = &(tmp_page->next);
        }

        *total_pages += NUM_PAD_PAGES;

        return 0;

err_free_page_malloc:
err_pad_page_write:
        forgetFreePages(PAD_PAGES_BEGIN);
        return 1;
}

extern unsigned insertPage(FILE *const fp, const unsigned long PAGE_NUM, const uint8_t *const PAGE_DATA, struct free_page **const free_pages, unsigned long *const total_pages){
        if(fseek(fp, HEADER_SIZE + PAGE_NUM*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page %lu.\n", PAGE_NUM);
                return 1;
        }

        if(!fwrite(PAGE_DATA, PAGE_SIZE, 1, fp)){
                fprintf(stderr, "Error writing page %lu.\n", PAGE_NUM);
                return 1;
        }

        removeFreePage(PAGE_NUM, free_pages);

        if(PAGE_NUM == *total_pages){
                (*total_pages)++;

                if(*total_pages <= MAX_PAGE_NUMBER){
                        if(insertFreePage(free_pages, *total_pages)){
                                return 1;
                        }
                }
        }

        return 0;
}

extern unsigned writeFicHeader(FILE *fp){
        if(!fwrite(MAGIC, sizeof(MAGIC), 1, fp)){
                fprintf(stderr, "Unable to write magic number.\n");
                return 1;
        }

        if(!fwrite(&VERSION, sizeof(VERSION), 1, fp)){
                fprintf(stderr, "Unable to write Interfic file version.\n");
                return 1;
        }

        return 0;
}

extern void writePageNumber(uint8_t *fic_page_num, const unsigned long PAGE_NUM){
        for(size_t i = 0; i < PAGE_NUM_SIZE; i++){
                fic_page_num[i] = PAGE_NUM >> (i*8);
        }
}
