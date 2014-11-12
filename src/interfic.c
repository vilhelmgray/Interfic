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

static unsigned addChoice(unsigned long *const next_page_num, FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *const selected_page, struct free_page **free_pages, unsigned long *total_pages);
static unsigned createNewFic(const char *const fLoc);
static unsigned createPage(FILE *const fp, const unsigned long PAGE_NUM, struct free_page **free_pages, unsigned long *total_pages);
static unsigned editFic(const char *const fLoc);
static unsigned editPage(unsigned long *const next_page_num, FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *const selected_page, struct free_page **free_pages, unsigned long *total_pages);
static unsigned modifyPages(FILE *const fp);
static unsigned performMenu(const char *const OPTIONS[], const size_t OPTIONS_SIZE);
static void printPage(const struct fic_page *const SELECTED_PAGE);
static unsigned readFic(const char *const fLoc);
static unsigned removeChoice(FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *const selected_page, struct free_page **free_pages, unsigned long *total_pages);
static unsigned long selectPageNumber(const struct free_page *const FREE_PAGES);

int main(void){
        const char *const FILE_MENU[] = { "Read file", "Edit existing file", "Create new file" };
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
                        if(readFic(fLoc)){
                                return 1;
                        }
                        break;
                case 2:
                        if(editFic(fLoc)){
                                return 1;
                        }
                        break;
                case 3:
                        if(createNewFic(fLoc)){
                                return 1;
                        }
                        break;
        }

        return 0;
}

static unsigned addChoice(unsigned long *const next_page_num, FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *const selected_page, struct free_page **free_pages, unsigned long *total_pages){
        size_t num_choices = 0;
        while(num_choices < MAX_NUM_CHOICES && selected_page->choice[num_choices].text[0]){
                num_choices++;
        }

        if(num_choices == MAX_NUM_CHOICES){
                printf("The is no more room on this page for another choice.\n");
                return 0;
        }

        printf("Enter Choice %zu text (maximum text length of %zu characters): ", num_choices+1, CHOICE_SIZE);
        char choice_text[CHOICE_SIZE+1] = "";
        fgets(choice_text, sizeof(choice_text), stdin);

        memcpy(selected_page->choice[num_choices].text, choice_text, CHOICE_SIZE);
        selected_page->choice[num_choices].page_num = selectPageNumber(*free_pages);

        if(writePage(fp, PAGE_NUM, selected_page, free_pages, total_pages)){
                return 1;
        }

        *next_page_num = selected_page->choice[num_choices].page_num;

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

        if(modifyPages(fp)){
                goto exit_modify_pages;
        }

        fclose(fp);
        return 0;

exit_modify_pages:
exit_header_write:
        fclose(fp);
        return 1;
}

static unsigned createPage(FILE *const fp, const unsigned long PAGE_NUM, struct free_page **free_pages, unsigned long *total_pages){
        printf("Enter page text (maximum text length of %zu characters): ", TEXT_SIZE);
        char page_text[TEXT_SIZE+1] = "";
        fgets(page_text, sizeof(page_text), stdin);

        struct fic_page new_page = {0};
        memcpy(new_page.text, page_text, TEXT_SIZE);

        if(PAGE_NUM > *total_pages){
                const unsigned long NUM_PAD_PAGES = PAGE_NUM - *total_pages;
                if(addPaddingPages(fp, *free_pages, total_pages, NUM_PAD_PAGES)){
                        return 1;
                }
        }

        if(writePage(fp, PAGE_NUM, &new_page, free_pages, total_pages)){
                return 1;
        }

        return 0;
}

static unsigned editFic(const char *const fLoc){
        FILE *const fp = fopen(fLoc, "r+b");
        if(!fp){
                fprintf(stderr, "Unable to open %s!\n", fLoc);
                return 1;
        }

        if(modifyPages(fp)){
                goto exit_modify_pages;
        }

        fclose(fp);
        return 0;

exit_modify_pages:
        fclose(fp);
        return 1;
}

static unsigned editPage(unsigned long *const next_page_num, FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *const selected_page, struct free_page **free_pages, unsigned long *total_pages){
        unsigned option = 1;
        if(selected_page->text[0]){
                printPage(selected_page);

                const char *const EDIT_MENU[] = { "Create new page", "Add new choice", "Remove a choice", "Remove page", "Exit menu" };
                option = performMenu(EDIT_MENU, sizeof(EDIT_MENU)/sizeof(*EDIT_MENU));
        }else{
                printf("Page %lu is empty.\n", PAGE_NUM);
        }

        switch(option){
                case 1:
                        if(createPage(fp, PAGE_NUM, free_pages, total_pages)){
                                return 1;
                        }
                        break;
                case 2:
                        if(addChoice(next_page_num, fp, PAGE_NUM, selected_page, free_pages, total_pages)){
                                return 1;
                        }
                        break;
                case 3:
                        if(removeChoice(fp, PAGE_NUM, selected_page, free_pages, total_pages)){
                                return 1;
                        }
                        break;
                case 4:
                        if(erasePage(fp, PAGE_NUM, free_pages)){
                                return 1;
                        }
                        break;
                default:
                        break;
        }

        return 0;
}

static unsigned modifyPages(FILE *const fp){
        struct free_page *free_pages;
        unsigned long total_pages;
        if(discoverFreePages(&free_pages, &total_pages, fp)){
                goto exit_free_pages_discovery;
        }

        unsigned option = 3;
        do{
                static unsigned long page_num = 0;
                if(option == 3){
                        page_num = selectPageNumber(free_pages);
                }

                struct fic_page selected_page = {0};
                if(page_num < total_pages){
                        if(readPage(fp, page_num, &selected_page)){
                                goto exit_page_selection;
                        }
                }

                unsigned long next_page_num = page_num;
                if(editPage(&next_page_num, fp, page_num, &selected_page, &free_pages, &total_pages)){
                        goto exit_page_modification;
                }

                if(next_page_num == page_num){
                        const char *const EXIT_MENU[] = { "Exit now", "Edit current page", "Choose a page" };
                        option = performMenu(EXIT_MENU, sizeof(EXIT_MENU)/sizeof(*EXIT_MENU));
                }else{
                        page_num = next_page_num;
                        option = 2;
                }
        }while(option != 1);

        forgetFreePages(free_pages);
        return 0;

exit_page_modification:
exit_page_selection:
        forgetFreePages(free_pages);
exit_free_pages_discovery:
        return 1;
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

static void printPage(const struct fic_page *const SELECTED_PAGE){
        char page_text[TEXT_SIZE+1] = "";
        memcpy(page_text, SELECTED_PAGE->text, TEXT_SIZE);
        printf("Page text:\n%s\n\n", page_text);

        for(size_t i = 0; i < MAX_NUM_CHOICES && SELECTED_PAGE->choice[i].text[0]; i++){
                char choice_text[CHOICE_SIZE+1] = "";
                memcpy(choice_text, SELECTED_PAGE->choice[i].text, CHOICE_SIZE);

                printf("\tChoice %zu: %s <Go to page %lu.>\n", i+1, choice_text, SELECTED_PAGE->choice[i].page_num);
        }

        putchar('\n');
}

static unsigned readFic(const char *const fLoc){
        FILE *const fp = fopen(fLoc, "rb");
        if(!fp){
                fprintf(stderr, "Unable to open %s!\n", fLoc);
                return 1;
        }

        unsigned option = 1;
        do{
                static unsigned long page_num = 0;
                if(option == 1){
                        do{
                                printf("Enter page number (0 - %lu): ", MAX_PAGE_NUMBER);
                                char buffer[32];
                                fgets(buffer, sizeof(buffer), stdin);
                                page_num = strtoul(buffer, NULL, 0);
                        }while(page_num > MAX_PAGE_NUMBER);
                }

                struct fic_page page = {0};
                if(readPage(fp, page_num, &page)){
                        goto exit_page_read;
                }

                if(page.text[0]){
                        printPage(&page);

                        size_t num_choices = 0;
                        while(num_choices < MAX_NUM_CHOICES && page.choice[num_choices].text[0]){
                                num_choices++;
                        }

                        if(num_choices){
                                size_t choice;
                                do{
                                        printf("Enter choice number (1 - %zu; 0 to exit page): ", num_choices);
                                        char buffer[32];
                                        fgets(buffer, sizeof(buffer), stdin);
                                        choice = strtoul(buffer, NULL, 0);
                                }while(choice > num_choices);

                                if(choice){
                                        page_num = page.choice[choice-1].page_num;
                                        option = 2;
                                        continue;
                                }
                        }
                }else{
                        printf("Page %lu is empty.\n", page_num);
                }

                const char *const END_MENU[] = { "Exit now", "Enter a specific page number" };
                option = performMenu(END_MENU, sizeof(END_MENU)/sizeof(*END_MENU)) - 1;
        }while(option);

        fclose(fp);
        return 0;

exit_page_read:
        fclose(fp);
        return 1;
}

static unsigned removeChoice(FILE *const fp, const unsigned long PAGE_NUM, struct fic_page *const selected_page, struct free_page **free_pages, unsigned long *total_pages){
        size_t num_choices = 0;
        while(num_choices < MAX_NUM_CHOICES && selected_page->choice[num_choices].text[0]){
                num_choices++;
        }

        if(!num_choices){
                printf("There are already no choices.\n");
                return 0;
        }

        size_t choice;
        do{
                printf("Enter the number of the choice you want removed (1 - %zu): ", num_choices);
                char buffer[32];
                fgets(buffer, sizeof(buffer), stdin);
                choice = strtoul(buffer, NULL, 0);
        }while(!choice || choice > num_choices);

        if(choice < num_choices){
                memmove(selected_page->choice + choice-1, selected_page->choice + choice, (num_choices-choice)*sizeof(*selected_page->choice));
        }
        selected_page->choice[num_choices-1].text[0] = 0;

        if(writePage(fp, PAGE_NUM, selected_page, free_pages, total_pages)){
                return 1;
        }

        return 0;
}

static unsigned long selectPageNumber(const struct free_page *const FREE_PAGES){
        const char *const SELECT_MENU[] = { "Go to a known free page", "Enter a specific page number" };
        unsigned option = performMenu(SELECT_MENU, sizeof(SELECT_MENU)/sizeof(*SELECT_MENU));

        unsigned long page_num;
        switch(option){
                case 1:
                        if(FREE_PAGES){
                                page_num = FREE_PAGES->page_num;
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

        return page_num;
}
