#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"


int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    char *filename  = NULL;
    FILE *f;

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
            "--file <filename in image>");
        exit(1);
    }
    
    // Open file
    f = fopen(imagename, "r");
    if (f == NULL) {
        fprintf(stderr, "image does not exist.\n");
        exit(1);
    }
    
    // Set superblock
    fread(&sb, sizeof(superblock_entry_t), 1, f);
    
    // Goto DIR block
    fseek(f, ntohl(sb.dir_start)*ntohs(sb.block_size), SEEK_SET);
    
    // Search DIR blocks for filename
    directory_entry_t de;
    char file_found = 0;
    for (i = 0; i < ntohl(sb.dir_blocks)*(ntohs(sb.block_size)/64); i++) {
        fread(&de, sizeof(directory_entry_t), 1, f);
        
        if (de.status == DIR_ENTRY_NORMALFILE && strcmp(de.filename, filename) == 0) {
            file_found = 1;
            fseek(f, ntohl(de.start_block)*ntohs(sb.block_size), SEEK_SET);
            // printf("start block: %d, block size %d\n", ntohl(de.start_block), ntohs(sb.block_size));
            char file[ntohl(de.file_size)];
            fread(&file, ntohl(de.file_size), 1, f);
            printf("%s", file);
        }
    }
    
    if (!file_found) {
        printf("file not found\n");
    }
    
    // Close the file
    fclose(f);

    return 0; 
}
