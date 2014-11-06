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
