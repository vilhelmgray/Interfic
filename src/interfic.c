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
static unsigned selectPage(unsigned free_page, unsigned *total_pages);

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
        FILE *fp = fopen(fLoc, "w+b");
        if(!fp){
                fprintf(stderr, "Unable to open %s!\n", fLoc);
                return 1;
        }

        if(writeFicHeader(fp)){
                goto exit_header_write;
        }

        unsigned total_pages = 0;
        if(selectPage(0, &total_pages)){
                goto exit_page_selection;
        }

        fclose(fp);
        return 0;

exit_page_selection:
exit_header_write:
        fclose(fp);
        return 1;
}

static unsigned selectPage(unsigned free_page, unsigned *total_pages){
        unsigned long page_num = 0;
        do{
                printf("Enter page number (0 - %lu) to create (page %u is free): ", MAX_PAGE_NUMBER, free_page);
                char buffer[32];
                fgets(buffer, sizeof(buffer), stdin);
                page_num = strtoul(buffer, NULL, 0);
        }while(page_num > MAX_PAGE_NUMBER);
        return 0;
}
