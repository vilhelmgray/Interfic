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
#include <stdlib.h>

#include "libinterfic.h"

static unsigned createNewFic(const char *const fLoc);
static unsigned createPage(struct free_page *free_pages, unsigned long *total_pages, FILE *const fp);

int main(void){
        unsigned choice = 0;
        do{
                printf("Select an option:\n"
                       "\t1. Open existing file\n"
                       "\t2. Create new file\n"
                       "> ");
                char buffer[32];
                fgets(buffer, sizeof(buffer), stdin);
                choice = strtoul(buffer, NULL, 0);
        }while(!choice || choice > 2);

        printf("Enter the file location: ");
        char fLoc[256];
        fgets(fLoc, sizeof(fLoc), stdin);
        size_t offset = 0;
        while(offset < sizeof(fLoc) && !iscntrl(fLoc[offset])){
                offset++;
        }
        fLoc[offset] = '\0';

        switch(choice){
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

        if(createPage(free_pages, &total_pages, fp)){
                goto exit_page_selection;
        }

        forgetFreePages(free_pages);

        fclose(fp);
        return 0;

exit_page_selection:
        forgetFreePages(free_pages);
exit_free_pages_discovery:
exit_header_write:
        fclose(fp);
        return 1;
}

static unsigned createPage(struct free_page *free_pages, unsigned long *total_pages, FILE *const fp){
        if(*total_pages <= MAX_PAGE_NUMBER){
                printf("Page %lu is free.\n", (free_pages) ? free_pages->page_num : *total_pages);
        }else{
                printf("There are no pages free.\n");
        }

        unsigned long page_num = 0;
        do{
                printf("Enter page number (0 - %lu) to create: ", MAX_PAGE_NUMBER);
                char buffer[32];
                fgets(buffer, sizeof(buffer), stdin);
                page_num = strtoul(buffer, NULL, 0);
        }while(page_num > MAX_PAGE_NUMBER);

        if(page_num > *total_pages){
                const unsigned long NUM_PAD_PAGES = page_num - *total_pages;
                if(addPaddingPages(fp, free_pages, *total_pages, NUM_PAD_PAGES)){
                        return 1;
                }
                *total_pages += NUM_PAD_PAGES;
        }

        printf("Enter page text (maximum text length of %zu characters): ", PAGE_SIZE);
        unsigned char page_data[PAGE_SIZE] = {0};
        fgets(page_data, TEXT_SIZE+1, stdin);

        if(insertPage(fp, page_num, page_data, &free_pages)){
                return 1;
        }

        return 0;
}
