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
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "free_pages.h"
#include "libinterfic.h"

static unsigned createNewFic(const char *const fLoc);
static unsigned createPage(FILE *const fp, const unsigned long PAGE_NUM, struct free_page **free_pages, unsigned long *total_pages);
static unsigned performMenu(const char *const OPTIONS[], const size_t OPTIONS_SIZE);

int main(void){
        const char *const FILE_MENU[] = { "Open existing file", "Create new file" };
        unsigned option = performMenu(FILE_MENU, sizeof(FILE_MENU)/sizeof(*FILE_MENU));

        printf("Enter the file location: ");
        char fLoc[256];
        fgets(fLoc, sizeof(fLoc), stdin);
        size_t offset = 0;
        while(offset < sizeof(fLoc) && !iscntrl(fLoc[offset])){
                offset++;
        }
        fLoc[offset] = '\0';

        switch(option){
                case 1:
                        break;
                case 2:
                        if(createNewFic(fLoc)){
                                return 1;
                        }
                        break;
        }

        return 0;
}

static unsigned createNewFic(const char *const fLoc){
        FILE *const fp = fopen(fLoc, "w+b");
        if(!fp){
                fprintf(stderr, "Unable to open %s!\n", fLoc);
                return 1;
        }

        if(writeFicHeader(fp)){
                goto exit_header_write;
        }

        struct free_page *free_pages;
        unsigned long total_pages;
        if(discoverFreePages(&free_pages, &total_pages, fp)){
                goto exit_free_pages_discovery;
        }

        const char *const SELECT_MENU[] = { "Use a known free page", "Enter a specific page number" };
        unsigned option = performMenu(SELECT_MENU, sizeof(SELECT_MENU)/sizeof(*SELECT_MENU));

        unsigned long page_num;
        switch(option){
                case 1:
                        if(free_pages){
                                page_num = free_pages->page_num;
                        }else{
                                printf("There are no pages free.\n");
                                page_num = MAX_PAGE_NUMBER;
                        }
                        printf("Page %lu selected.\n", page_num);
                        break;
                case 2:
                        do{
                                printf("Enter page number (0 - %lu): ", MAX_PAGE_NUMBER);
                                char buffer[32];
                                fgets(buffer, sizeof(buffer), stdin);
                                page_num = strtoul(buffer, NULL, 0);
                        }while(page_num > MAX_PAGE_NUMBER);
                        break;
        }

        uint8_t page_data[PAGE_SIZE] = {0};
        if(page_num < total_pages){
                if(lookupPage(fp, page_num, page_data)){
                        goto exit_page_selection;
                }
        }

        if(!page_data[0]){
                printf("Page %lu is empty.\n", page_num);
        }

        const char *const EDIT_MENU[] = { "Create new page", "Add new choice" };
        option = performMenu(EDIT_MENU, sizeof(EDIT_MENU)/sizeof(*EDIT_MENU));

        switch(option){
                case 1:
                        if(createPage(fp, page_num, &free_pages, &total_pages)){
                                goto exit_page_creation;
                        }
                        break;
                case 2:
                        break;
        }

        forgetFreePages(free_pages);

        fclose(fp);
        return 0;

exit_page_creation:
exit_page_selection:
        forgetFreePages(free_pages);
exit_free_pages_discovery:
exit_header_write:
        fclose(fp);
        return 1;
}

static unsigned createPage(FILE *const fp, const unsigned long PAGE_NUM, struct free_page **free_pages, unsigned long *total_pages){
        printf("Enter page text (maximum text length of %zu characters): ", PAGE_SIZE);
        char page_text[TEXT_SIZE+1] = "";
        fgets(page_text, sizeof(page_text), stdin);

        uint8_t page_data[PAGE_SIZE] = {0};
        memcpy(page_data, page_text, TEXT_SIZE);

        unsigned num_choices;
        do{
                printf("Enter the number of choices (0 - %u): ", MAX_NUM_CHOICES);
                char buffer[32];
                fgets(buffer, sizeof(buffer), stdin);
                num_choices = strtoul(buffer, NULL, 0);
        }while(num_choices > MAX_NUM_CHOICES);

        for(unsigned i = 0; i < num_choices; i++){
                printf("Enter Choice %u text (maximum text length of %zu characters): ", i, CHOICE_SIZE);
                const size_t CHOICE_OFFSET = TEXT_SIZE + i*(CHOICE_SIZE+PAGE_NUM_SIZE);
                fgets(page_data + CHOICE_OFFSET, CHOICE_SIZE+1, stdin);

                unsigned long choice_page_num;
                do{
                        printf("Enter Choice %u page number (0 - %lu): ", i, MAX_PAGE_NUMBER);
                        char buffer[32];
                        fgets(buffer, sizeof(buffer), stdin);
                        choice_page_num = strtoul(buffer, NULL, 0);
                }while(choice_page_num > MAX_PAGE_NUMBER);
                writePageNumber(page_data + CHOICE_OFFSET + CHOICE_SIZE, choice_page_num);
        }

        if(PAGE_NUM > *total_pages){
                const unsigned long NUM_PAD_PAGES = PAGE_NUM - *total_pages;
                if(addPaddingPages(fp, *free_pages, total_pages, NUM_PAD_PAGES)){
                        return 1;
                }
        }

        if(insertPage(fp, PAGE_NUM, page_data, free_pages, total_pages)){
                return 1;
        }

        return 0;
}

static unsigned performMenu(const char *const OPTIONS[], const size_t OPTIONS_SIZE){
        unsigned option;
        do{
                printf("Select an option:\n");
                for(size_t i = 0; i < OPTIONS_SIZE; i++){
                        printf("\t%zu. %s\n", i+1, OPTIONS[i]);
                }
                printf("> ");
                char buffer[32];
                fgets(buffer, sizeof(buffer), stdin);
                option = strtoul(buffer, NULL, 0);
        }while(!option || option > OPTIONS_SIZE);

        return option;
}
