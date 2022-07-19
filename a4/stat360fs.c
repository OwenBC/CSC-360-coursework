#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"

char *basename(char *path) {
    char *base = NULL;
    char *cur = strchr(path, '/');
    while (cur != NULL) {
        base = cur+1;
        cur = strchr(cur + 1, '/');
    }
    
    return base;
}

int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    FILE  *f;
    int   fat_data;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL){
        fprintf(stderr, "usage: stat360fs --image <imagename>\n");
        exit(1);
    }
    
    // Open file
    f = fopen(imagename, "r");
    if (f == NULL) {
        fprintf(stderr, "file does not exist.\n");
        exit(1);
    }
    
    // Set superblock
    fread(&sb, sizeof(superblock_entry_t), 1, f);
    
    // Read FAT and count resv/alloc
    int Resv = 0;
    int Alloc = 0;
    
    fseek(f, ntohl(sb.fat_start)*ntohs(sb.block_size), SEEK_SET);
    
    fread(&fat_data, 4, 1, f);
    fat_data = ntohl(fat_data);
    while (fat_data != 0) {
        if (fat_data == 1) Resv++;
        else Alloc++;
        fread(&fat_data, 4, 1, f);
        fat_data = ntohl(fat_data);
    }
    
    fclose(f);
    
    // Output
    char *line = "-------------------------------------------------\n";
    printf("%s (%s)\n\n%s", sb.magic, basename(imagename), line);
    printf("  Bsz   Bcnt  FATst FATcnt  DIRst DIRcnt\n%5d %6d %6d %6d %6d %6d\n\n%s", 
           ntohs(sb.block_size), ntohl(sb.num_blocks), ntohl(sb.fat_start), 
           ntohl(sb.fat_blocks), ntohl(sb.dir_start), ntohl(sb.dir_blocks), line);
    printf(" Free   Resv  Alloc\n%5d %6d %6d\n\n", 
           ntohl(sb.num_blocks) - (Resv + Alloc), 
           Resv, 
           Alloc);

    return 0; 
}
