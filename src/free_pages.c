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

#include "free_pages.h"
#include "libinterfic.h"

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

extern unsigned insertFreePage(struct free_page **head_page, const unsigned long PAGE_NUM){
        struct free_page *new_free_page = malloc(sizeof(*new_free_page));
        if(!new_free_page){
                fprintf(stderr, "Unable to allocate memory for free pages list.\n");
                return 1;
        }
        new_free_page->page_num = PAGE_NUM;
        new_free_page->next = NULL;

        while(*head_page){
                struct free_page *cur_page = *head_page;
                if(cur_page->page_num > PAGE_NUM){
                        new_free_page->next = cur_page;
                        break;
                }

                head_page = &(cur_page->next);
        }

        *head_page = new_free_page;

        return 0;
}

extern void removeFreePage(const unsigned long PAGE_NUM, struct free_page **head){
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
