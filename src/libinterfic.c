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

#include "libinterfic.h"

static const uint8_t MAGIC[] = { 0x49, 0x4E, 0x54, 0x45, 0x52, 0x46, 0x49, 0x43 };
static const uint8_t VERSION = 0;

#define MAX_OFFSET      ((1UL<<31) - 1)
#define HEADER_SIZE     (sizeof(MAGIC) + sizeof(VERSION))
#define MAX_FIC_SIZE    (MAX_OFFSET + PAGE_SIZE - HEADER_SIZE)
const unsigned long MAX_PAGE_NUMBER = MAX_FIC_SIZE/PAGE_SIZE - 1;

static void removeFreePage(const unsigned long PAGE_NUM, struct free_page **head);

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, const unsigned long TOTAL_PAGES, const unsigned long NUM_PAD_PAGES){
        if(fseek(fp, HEADER_SIZE + TOTAL_PAGES*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to end of file.\n");
                return 1;
        }

        struct free_page **free_pages_end = &free_pages;
        while(*free_pages_end){
                free_pages_end = &((*free_pages_end)->next);
        }
        struct free_page *const PAD_PAGES_BEGIN = *free_pages_end;

        for(unsigned long i = TOTAL_PAGES; i < TOTAL_PAGES + NUM_PAD_PAGES; i++){
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

        return 0;

err_free_page_malloc:
err_pad_page_write:
        forgetFreePages(PAD_PAGES_BEGIN);
        return 1;
}

extern unsigned discoverFreePages(struct free_page **const free_pages, unsigned long *const total_pages, FILE *const fp){
        if(fseek(fp, HEADER_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page 0.\n");
                return 1;
        }

        unsigned isEOF = 0;
        unsigned long page_num = 0;
        struct free_page **next_free_page = free_pages;
        do{
                char page_data[PAGE_SIZE];
                fread(page_data, sizeof(page_data), 1, fp);
                if(feof(fp)){
                        page_data[0] = '\0';
                        isEOF = 1;
                }else if(ferror(fp)){
                        fprintf(stderr, "Error trying to read page %lu.\n", page_num);
                        return 1;
                }

                if(!page_data[0]){
                        struct free_page *tmp_page = malloc(sizeof(*tmp_page));
                        if(!tmp_page){
                                fprintf(stderr, "Unable to allocate memory for free pages list.\n");
                                goto err_free_page_malloc;
                        }
                        tmp_page->page_num = page_num;
                        tmp_page->next = NULL;

                        *next_free_page = tmp_page;
                        next_free_page = &(tmp_page->next);
                }

                page_num++;
        }while(!isEOF && page_num <= MAX_PAGE_NUMBER);

        *total_pages = (isEOF) ? page_num - 1 : page_num;
        return 0;

err_free_page_malloc:
        forgetFreePages(*free_pages);
        return 1;
}

extern void forgetFreePages(struct free_page *free_pages){
        while(free_pages){
                struct free_page *next = free_pages->next;
                free(free_pages);
                free_pages = next;
        }
}

extern unsigned insertPage(FILE *const fp, const unsigned long PAGE_NUM, const uint8_t *const PAGE_DATA, struct free_page **const free_pages){
        if(fseek(fp, HEADER_SIZE + PAGE_NUM*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page %lu.\n", PAGE_NUM);
                return 1;
        }

        if(!fwrite(PAGE_DATA, PAGE_SIZE, 1, fp)){
                fprintf(stderr, "Error writing page %lu.\n", PAGE_NUM);
                return 1;
        }

        removeFreePage(PAGE_NUM, free_pages);

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

static void removeFreePage(const unsigned long PAGE_NUM, struct free_page **head){
        struct free_page *cur_page = *head;
        while(cur_page){
                if(cur_page->page_num == PAGE_NUM){
                        *head = cur_page->next;

                        free(cur_page);
                        break;
                }

                head = &(cur_page->next);
                cur_page = *head;
        }
}
