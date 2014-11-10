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
#include <string.h>

#include "free_pages.h"
#include "libinterfic.h"

static const uint8_t MAGIC[8] = { 0x49, 0x4E, 0x54, 0x45, 0x52, 0x46, 0x49, 0x43 };
static const uint8_t VERSION = 0;

#define MAX_OFFSET      ((1UL<<31) - 1)
#define MAX_FIC_SIZE    (MAX_OFFSET + PAGE_SIZE - HEADER_SIZE)
const unsigned long MAX_PAGE_NUMBER = MAX_FIC_SIZE/PAGE_SIZE - 1;

static unsigned long readPageNumber(const uint8_t *const FIC_PAGE_NUM);

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

                if(insertFreePage(free_pages_end, i)){
                        goto err_free_page_malloc;
                }
                free_pages_end = &((*free_pages_end)->next);
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

extern unsigned readPage(FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *read_page){
        if(fseek(fp, HEADER_SIZE + PAGE_NUM*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page %lu.\n", PAGE_NUM);
                return 1;
        }

        uint8_t page_data[PAGE_SIZE];
        if(!fread(page_data, PAGE_SIZE, 1, fp)){
                fprintf(stderr, "Error reading page %lu.\n", PAGE_NUM);
                return 1;
        }

        memcpy(read_page->text, page_data, TEXT_SIZE);
        for(size_t i = 0; i < MAX_NUM_CHOICES; i++){
                const size_t CHOICE_OFFSET = TEXT_SIZE + i*(CHOICE_SIZE + PAGE_NUM_SIZE);
                memcpy(read_page->choice[i].text, page_data + CHOICE_OFFSET, CHOICE_SIZE);

                read_page->choice[i].page_num = readPageNumber(page_data + CHOICE_OFFSET + CHOICE_SIZE);
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

static unsigned long readPageNumber(const uint8_t *const FIC_PAGE_NUM){
        unsigned long page_num = 0;
        for(size_t i = 0; i < PAGE_NUM_SIZE; i++){
                page_num |= (unsigned long)FIC_PAGE_NUM[i] << i*8;
        }

        return page_num;
}
