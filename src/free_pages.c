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

#include <stdlib.h>

#include "free_pages.h"

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
