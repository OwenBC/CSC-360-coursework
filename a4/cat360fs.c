#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i, ii, iii;
    char *imagename = NULL;
    char *filename  = NULL;
    FILE *f;
    int   fat_data;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL || filename == NULL) {
        fprintf(stderr, "usage: cat360fs --image <imagename> " \
            "--file <filename in image>\n");
        exit(1);
    }
    
    // Open file
    f = fopen(imagename, "rb");
    if (f == NULL) {
        fprintf(stderr, "image does not exist.\n");
        exit(1);
    }
    
    // Set superblock
    fread(&sb, sizeof(superblock_entry_t), 1, f);
    short block_size = ntohs(sb.block_size);
    int dir_start = ntohl(sb.dir_start);
    int fat_start = ntohl(sb.fat_start);
    
    // Goto DIR block
    fseek(f, dir_start*block_size, SEEK_SET);
    
    // Search DIR blocks for filename
    directory_entry_t de;
    char file_found = 0;
    
    for (i = 0; i < MAX_DIR_ENTRIES; i++) {
        fread(&de, sizeof(directory_entry_t), 1, f);
        
        if (de.status == DIR_ENTRY_NORMALFILE && strcmp(de.filename, filename) == 0) {
            // If file is found...
            file_found = 1;
            int start_block = ntohl(de.start_block);
            int num_blocks = ntohl(de.num_blocks);
            int data_block = start_block;
            char byte = 0;
            
            for (ii = 0; ii < num_blocks; ii++) { 
                // Print block
                fseek(f, data_block*block_size, SEEK_SET);
                for(iii = 0; iii < (block_size/sizeof(char)); iii++) {
                    fread(&byte, sizeof(char), 1, f);
                    printf("%c", byte);
                }
                
                // Find next block
                fseek(f, (fat_start*block_size)+(data_block*4), SEEK_SET);
                fread(&fat_data, 4, 1, f);
                data_block = ntohl(fat_data);
            }
        }
    }
    
    if (!file_found) {printf("file not found\n");}
    
    // Close the file
    fclose(f);

    return 0; 
}
