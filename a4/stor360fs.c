#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "disk.h"

void *emalloc(size_t n) {
    void *p;

    p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "malloc of %zu bytes failed", n);
        exit(1);
    }

    return p;
}

/*
 * Based on http://bit.ly/2vniWNb
 */
void pack_current_datetime(unsigned char *entry) {
    assert(entry);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    unsigned short year   = tm.tm_year + 1900;
    unsigned char  month  = (unsigned char)(tm.tm_mon + 1);
    unsigned char  day    = (unsigned char)(tm.tm_mday);
    unsigned char  hour   = (unsigned char)(tm.tm_hour);
    unsigned char  minute = (unsigned char)(tm.tm_min);
    unsigned char  second = (unsigned char)(tm.tm_sec);

    year = htons(year);

    memcpy(entry, &year, 2);
    entry[2] = month;
    entry[3] = day;
    entry[4] = hour;
    entry[5] = minute;
    entry[6] = second; 
}


int next_free_block(int *FAT, int max_blocks) {
    assert(FAT != NULL);

    int i;

    for (i = 0; i < max_blocks; i++) {
        if (FAT[i] == FAT_AVAILABLE) {
            return i;
        }
    }

    return -1;
}


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename  = NULL;
    char *filename   = NULL;
    char *sourcename = NULL;
    FILE *img, *src;
    int  *fat_data;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--source") == 0 && i+1 < argc) {
            sourcename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL || filename == NULL || sourcename == NULL) {
        fprintf(stderr, "usage: stor360fs --image <imagename> " \
            "--file <filename in image> " \
            "--source <filename on host>\n");
        exit(1);
    }

    // Open image file
    img = fopen(imagename, "r+b");
    if (img == NULL) {
        fprintf(stderr, "image does not exist.\n");
        exit(1);
    }
    // Open host file
    src = fopen(sourcename, "rb");
    if (src == NULL) {
        fprintf(stderr, "source file does not exist.\n");
        exit(1);
    }
    
    // Set superblock
    fread(&sb, sizeof(superblock_entry_t), 1, img);
    short block_size = ntohs(sb.block_size);
    int num_blocks = ntohl(sb.num_blocks);
    int fat_start = ntohl(sb.fat_start);
    int fat_blocks = ntohl(sb.fat_blocks);
    int fat_entries = fat_blocks*(block_size/SIZE_FAT_ENTRY);
    int dir_start = ntohl(sb.dir_start);
    
    
    // Exit if file exists on image
    // Goto DIR block
    fseek(img, dir_start*block_size, SEEK_SET);
    
    // Search DIR blocks for filename
    directory_entry_t de;
    char available_de = 0;
    for (i = 0; i < MAX_DIR_ENTRIES; i++) {
        fread(&de, sizeof(directory_entry_t), 1, img);
        
        if (de.status == DIR_ENTRY_NORMALFILE && strcmp(de.filename, filename) == 0) {
            // If file is found...
            fprintf(stderr, "filename (%s) already exists in image.\n", filename);
            exit(1);
        } else if (de.status == DIR_ENTRY_AVAILABLE) {
            available_de = 1;
        }
    }    
    
    // Exit if no available directory entries
    if (!available_de) {
        fprintf(stderr, "no space for new files, all directory entries (%d/%d) are in use.\n", MAX_DIR_ENTRIES, MAX_DIR_ENTRIES);
        exit(1);
    }
    
    // Get source file stats
    struct stat stat_block;
    if (stat(sourcename, &stat_block) == -1) {
        fprintf(stderr, "source file stats failed to load.\n");
        exit(1);
    }
    
    // Set FAT data
    fseek(img, fat_start*block_size, SEEK_SET);
    fat_data = emalloc(fat_entries*4);
    fread(fat_data, SIZE_FAT_ENTRY, fat_entries, img);
    
    // Calculate free blocks
    int free_blocks = num_blocks;
    for (i = 0; i < fat_entries; i++) {
        if (fat_data[i] != FAT_AVAILABLE) free_blocks--;
    }
    
    // Exit if there isn't enough space for host file 
    if (free_blocks*block_size < stat_block.st_size) {
        fprintf(stderr, "the filesystem image doesn't have have enough free space to store %s.\n", filename);
        exit(1);
    }
    
    // Store file
    char block[block_size];
    int bytes_read = fread(block, sizeof(char), block_size, src);
    int de_start_block = next_free_block(fat_data, fat_entries);
    int de_num_blocks = 0;
    int cur_block = de_start_block;
    int next_block;
    
    while(bytes_read != 0) {
        // Write block
        fseek(img, cur_block*block_size, SEEK_SET);
        fwrite(block, sizeof(char), bytes_read, img);
        de_num_blocks++;
        
        // Read source file
        bytes_read = fread(block, sizeof(char), block_size, src);
        
        // Update FAT
        if (bytes_read) {
            fat_data[cur_block] = FAT_RESERVED;
            next_block = next_free_block(fat_data, fat_entries);
            fat_data[cur_block] = htonl(next_block);
            cur_block = next_block;
        } else {
            fat_data[cur_block] = FAT_LASTBLOCK;
        }
    }
    
    // Write updated FAT data
    fseek(img, fat_start*block_size, SEEK_SET);
    fwrite(fat_data, SIZE_FAT_ENTRY, fat_entries, img);
    
    // Create directory entry
    directory_entry_t new_de;
    new_de.status = DIR_ENTRY_NORMALFILE;
    new_de.start_block = htonl(de_start_block);
    new_de.num_blocks = htonl(de_num_blocks);
    new_de.file_size = htonl(stat_block.st_size);

    unsigned char datetime[DIR_TIME_WIDTH];
    pack_current_datetime(datetime);

    memcpy(new_de.create_time, datetime, DIR_TIME_WIDTH);
    memcpy(new_de.modify_time, datetime, DIR_TIME_WIDTH);
    strcpy(new_de.filename, filename);
    
    // Goto DIR block
    fseek(img, dir_start*block_size, SEEK_SET);
    // Find available directory
    for (i = 0; i < MAX_DIR_ENTRIES; i++) {
        fread(&de, sizeof(directory_entry_t), 1, img);
        
        if (de.status == DIR_ENTRY_AVAILABLE) {
            fseek(img, -SIZE_DIR_ENTRY, SEEK_CUR);
            break;
        }
    }
    // Write directory entry
    fwrite(&new_de, sizeof(directory_entry_t), 1, img);
    
    // Deallocate
    free(fat_data);
    
    // Close the files
    fclose(img);
    fclose(src);
    
    return 0; 
}
