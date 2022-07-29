#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
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
    FILE *f;
    int  *fat_data;

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
    f = fopen(imagename, "rb");
    if (f == NULL) {
        fprintf(stderr, "image does not exist.\n");
        exit(1);
    }
    
    // Set superblock
    fread(&sb, sizeof(superblock_entry_t), 1, f);
    
    // Read FAT and count resv/alloc
    int Resv = 0;
    int Alloc = 0;
    
    fseek(f, ntohl(sb.fat_start)*ntohs(sb.block_size), SEEK_SET);
    
    int fat_entries = ntohl(sb.fat_blocks)*(ntohs(sb.block_size)/SIZE_FAT_ENTRY);
    fat_data = emalloc(fat_entries*4);
    fread(fat_data, SIZE_FAT_ENTRY, fat_entries, f);
    for (i = 0; i < fat_entries; i++){
        if (ntohl(fat_data[i]) == FAT_RESERVED) Resv++;
        else if (fat_data[i] != FAT_AVAILABLE) Alloc++;
    }
    
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

    // Close the file
    fclose(f);
    
    return 0; 
}
