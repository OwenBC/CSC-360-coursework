#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include "disk.h"


char *month_to_string(short m) {
    switch(m) {
    case 1: return "Jan";
    case 2: return "Feb";
    case 3: return "Mar";
    case 4: return "Apr";
    case 5: return "May";
    case 6: return "Jun";
    case 7: return "Jul";
    case 8: return "Aug";
    case 9: return "Sep";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
    default: return "?!?";
    }
}


void unpack_datetime(unsigned char *time, short *year, short *month, 
    short *day, short *hour, short *minute, short *second)
{
    assert(time != NULL);

    memcpy(year, time, 2);
    *year = htons(*year);

    *month = (unsigned short)(time[2]);
    *day = (unsigned short)(time[3]);
    *hour = (unsigned short)(time[4]);
    *minute = (unsigned short)(time[5]);
    *second = (unsigned short)(time[6]);
}

void printlisting(int file_size, unsigned char *time, char *filename) {
    short year, month, day, hour, minute, second;
    unpack_datetime( time, &year, &month, &day, &hour, &minute, &second);
    printf("%8d %d-%s-%d %2d:%2d:%2d %s\n", file_size, year, month_to_string(month), 
           day, hour, minute, second, filename);
}

int main(int argc, char *argv[]) {
    superblock_entry_t sb;
    int  i;
    char *imagename = NULL;
    FILE *f;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagename = argv[i+1];
            i++;
        }
    }

    if (imagename == NULL)
    {
        fprintf(stderr, "usage: ls360fs --image <imagename>\n");
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
    
    // Read DIR blocks
    directory_entry_t de;
    for (i = 0; i < ntohl(sb.dir_blocks)*(ntohs(sb.block_size)/64); i++) {
        fread(&de, sizeof(directory_entry_t), 1, f);
        
        if (de.status == DIR_ENTRY_NORMALFILE) {
            printlisting(ntohl(de.file_size), de.modify_time, de.filename);
        }
    }
    
    // Close the file
    fclose(f);

    return 0; 
}
