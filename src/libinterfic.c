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
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "free_pages.h"
#include "libinterfic.h"

static const uint8_t MAGIC[8] = { 0x49, 0x4E, 0x54, 0x45, 0x52, 0x46, 0x49, 0x43 };
static const uint8_t VERSION = 0;

#define MAX_OFFSET      ((1UL<<31) - 1)
#define MAX_FIC_SIZE    (MAX_OFFSET + PAGE_SIZE - HEADER_SIZE)
const unsigned long MAX_PAGE_NUMBER = MAX_FIC_SIZE/PAGE_SIZE - 1;

static unsigned long readPageNumber(const uint8_t *const FIC_PAGE_NUM);
static void writePageNumber(uint8_t *fic_page_num, const unsigned long PAGE_NUM);

extern unsigned addPaddingPages(FILE *const fp, struct free_page *free_pages, unsigned long *const total_pages, const unsigned long NUM_PAD_PAGES){
        if(fseek(fp, HEADER_SIZE + (*total_pages)*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to end of file.\n");
                return 1;
        }

        struct free_page **free_pages_end = &free_pages;
        while(*free_pages_end){
                free_pages_end = &((*free_pages_end)->next);
        }
        struct free_page *const PAD_PAGES_BEGIN = *free_pages_end;

        for(unsigned long i = *total_pages; i < *total_pages + NUM_PAD_PAGES; i++){
                const unsigned char PAD_PAGE[PAGE_SIZE] = {'\0'};
                if(!fwrite(PAD_PAGE, sizeof(PAD_PAGE), 1, fp)){
                        fprintf(stderr, "Error writing padding pages.\n");
                        goto err_pad_page_write;
                }

                if(insertFreePage(free_pages_end, i)){
                        goto err_free_page_malloc;
                }
                free_pages_end = &((*free_pages_end)->next);
        }

        *total_pages += NUM_PAD_PAGES;

        return 0;

err_free_page_malloc:
err_pad_page_write:
        forgetFreePages(PAD_PAGES_BEGIN);
        return 1;
}

extern unsigned erasePage(FILE *const fp, const unsigned long PAGE_NUM, struct free_page **const free_pages){
        if(fseek(fp, HEADER_SIZE + PAGE_NUM*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page %lu.\n", PAGE_NUM);
                return 1;
        }

        if(fputc(0, fp)){
                fprintf(stderr, "Error erasing page %lu.\n", PAGE_NUM);
                return 1;
        }

        if(insertFreePage(free_pages, PAGE_NUM)){
                return 1;
        }

        return 0;
}

extern unsigned loadFicFile(FILE *const fp, unsigned long *const total_pages, struct free_page **const free_pages){
        if(fseek(fp, HEADER_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page 0.\n");
                return 1;
        }

        unsigned isEOF = 0;
        unsigned long page_num = 0;
        struct free_page **next_free_page = free_pages;
        do{
                char page_data[PAGE_SIZE];
                if(!fread(page_data, sizeof(page_data), 1, fp)){
                        if(feof(fp)){
                                page_data[0] = '\0';
                                isEOF = 1;
                        }else if(ferror(fp)){
                                fprintf(stderr, "Error trying to read page %lu.\n", page_num);
                                return 1;
                        }
                }

                if(free_pages && !page_data[0]){
                        if(insertFreePage(next_free_page, page_num)){
                                goto err_insert_free_page;
                        }

                        next_free_page = &((*next_free_page)->next);
                }

                page_num++;
        }while(!isEOF && page_num <= MAX_PAGE_NUMBER);

        *total_pages = (isEOF) ? page_num - 1 : page_num;
        return 0;

err_insert_free_page:
        forgetFreePages(*free_pages);
        return 1;
}

extern unsigned readPage(FILE *const fp, struct fic_page *read_page){
        if(fseek(fp, HEADER_SIZE + read_page->num*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page %lu.\n", read_page->num);
                return 1;
        }

        uint8_t page_data[PAGE_SIZE];
        if(!fread(page_data, PAGE_SIZE, 1, fp)){
                fprintf(stderr, "Error reading page %lu.\n", read_page->num);
                return 1;
        }

        memcpy(read_page->text, page_data, TEXT_SIZE);
        for(size_t i = 0; i < MAX_NUM_CHOICES; i++){
                const size_t CHOICE_OFFSET = TEXT_SIZE + i*(CHOICE_SIZE + PAGE_NUM_SIZE);
                memcpy(read_page->choice[i].text, page_data + CHOICE_OFFSET, CHOICE_SIZE);

                read_page->choice[i].page_num = readPageNumber(page_data + CHOICE_OFFSET + CHOICE_SIZE);
        }

        return 0;
}

extern unsigned writeFicHeader(FILE *fp){
        if(!fwrite(MAGIC, sizeof(MAGIC), 1, fp)){
                fprintf(stderr, "Unable to write magic number.\n");
                return 1;
        }

        if(fputc(VERSION, fp) == EOF){
                fprintf(stderr, "Unable to write Interfic file version.\n");
                return 1;
        }

        return 0;
}

extern unsigned writePage(FILE *const fp, const struct fic_page *const NEW_PAGE, struct free_page **const free_pages, unsigned long *const total_pages){
        if(fseek(fp, HEADER_SIZE + NEW_PAGE->num*PAGE_SIZE, SEEK_SET)){
                fprintf(stderr, "Error seeking to page %lu.\n", NEW_PAGE->num);
                return 1;
        }

        uint8_t page_data[PAGE_SIZE] = {0};
        memcpy(page_data, NEW_PAGE->text, TEXT_SIZE);
        for(size_t i = 0; i < MAX_NUM_CHOICES; i++){
                const size_t CHOICE_OFFSET = TEXT_SIZE + i*(CHOICE_SIZE + PAGE_NUM_SIZE);
                memcpy(page_data + CHOICE_OFFSET, NEW_PAGE->choice[i].text, CHOICE_SIZE);

                writePageNumber(page_data + CHOICE_OFFSET + CHOICE_SIZE, NEW_PAGE->choice[i].page_num);
        }

        if(!fwrite(page_data, PAGE_SIZE, 1, fp)){
                fprintf(stderr, "Error writing page %lu.\n", NEW_PAGE->num);
                return 1;
        }

        removeFreePage(NEW_PAGE->num, free_pages);

        if(NEW_PAGE->num == *total_pages){
                (*total_pages)++;

                if(*total_pages <= MAX_PAGE_NUMBER){
                        if(insertFreePage(free_pages, *total_pages)){
                                return 1;
                        }
                }
        }

        return 0;
}

extern unsigned verifyFicHeader(FILE *const fp){
        uint8_t file_magic[sizeof(MAGIC)];
        if(!fread(file_magic, sizeof(file_magic), 1, fp)){
                fprintf(stderr, "Unable to read magic number.\n");
                return 1;
        }

        if(memcmp(file_magic, MAGIC, sizeof(MAGIC))){
                printf("The magic number of the file does not match the expected Interfic magic number.\n");
                return 1;
        }

        int retval = fgetc(fp);
        if(retval == EOF){
                fprintf(stderr, "Unable to read Interfic file version.\n");
                return 1;
        }
        uint8_t file_version = retval;

        if(file_version > VERSION){
                printf("FIC file version is %" PRIu8 "; "
                       "this version of Interfic does not support FIC file versions greater than %" PRIu8 ".",
                       file_version, VERSION);
                return 1;
        }

        return 0;
}

static unsigned long readPageNumber(const uint8_t *const FIC_PAGE_NUM){
        unsigned long page_num = 0;
        for(size_t i = 0; i < PAGE_NUM_SIZE; i++){
                page_num |= (unsigned long)FIC_PAGE_NUM[i] << i*8;
        }

        return page_num;
}

static void writePageNumber(uint8_t *fic_page_num, const unsigned long PAGE_NUM){
        for(size_t i = 0; i < PAGE_NUM_SIZE; i++){
                fic_page_num[i] = PAGE_NUM >> (i*8);
        }
}
