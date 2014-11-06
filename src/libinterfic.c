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
#include <stdio.h>
#include <stdlib.h>

#include "libinterfic.h"

static const unsigned char MAGIC[] = { 0x49, 0x4E, 0x54, 0x45, 0x52, 0x46, 0x49, 0x43 };
static const unsigned char VERSION = 0;

#define MAX_OFFSET      ((1UL<<31) - 1)
#define HEADER_SIZE     (sizeof(MAGIC) + sizeof(VERSION))
#define TEXT_SIZE       1024
#define CHOICE_SIZE     256
#define PAGE_NUM_SIZE   2
#define PAGE_SIZE       (TEXT_SIZE + 4*(CHOICE_SIZE + PAGE_NUM_SIZE))
#define MAX_FIC_SIZE    (MAX_OFFSET + PAGE_SIZE - HEADER_SIZE)
const unsigned long MAX_PAGE_NUMBER = MAX_FIC_SIZE/PAGE_SIZE - 1;

extern unsigned discoverFreePages(struct free_page **const free_pages, unsigned long *const total_pages, FILE *const fp){
        if(fseek(fp, HEADER_SIZE, SEEK_SET)){
                struct free_page *tmp_page = calloc(1, sizeof(*tmp_page));
                if(!tmp_page){
                        fprintf(stderr, "Unable to allocate memory for free pages list\n");
                        return 1;
                }
                *free_pages = tmp_page;
                *total_pages = 0;
                return 0;
        }

        unsigned isEOF = 0;
        unsigned long page_num = 0;
        struct free_page **next_free_page = free_pages;
        do{
                char page_data[PAGE_SIZE] = {0};
                fread(page_data, sizeof(page_data), 1, fp);
                if(feof(fp)){
                        page_data[0] = '\0';
                        isEOF = 1;
                }else if(ferror(fp)){
                        fprintf(stderr, "Error trying to read page %lu\n", page_num);
                        return 1;
                }

                if(!page_data[0]){
                        struct free_page *tmp_page = malloc(sizeof(*tmp_page));
                        if(!tmp_page){
                                fprintf(stderr, "Unable to allocate memory for free pages list\n");
                                goto exit_page_malloc;
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

exit_page_malloc:
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

extern unsigned writeFicHeader(FILE *fp){
        if(!fwrite(MAGIC, sizeof(MAGIC), 1, fp)){
                fprintf(stderr, "Unable to write magic number\n");
                return 1;
        }

        if(!fwrite(&VERSION, sizeof(VERSION), 1, fp)){
                fprintf(stderr, "Unable to write Interfic file version\n");
                return 1;
        }

        if(fflush(fp)){
                fprintf(stderr, "Error flushing Interfic header to file\n");
                return 1;
        }

        return 0;
}
