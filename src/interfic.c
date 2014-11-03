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
#include <stdio.h>
#include <stdlib.h>

#include "libinterfic.h"

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

        FILE *fp;
        switch(choice){
                case 1:
                        fp = fopen(fLoc, "rb");
                        break;
                case 2:
                        fp = fopen(fLoc, "w+b");
                        break;
        }
        if(!fp){
                fprintf(stderr, "Unable to open %s!\n", fLoc);
                return 1;
        }

        fclose(fp);
        return 0;
}
