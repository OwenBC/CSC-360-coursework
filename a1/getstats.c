/* getstats.c 
 *
 * CSC 360, Summer 2022
 *
 * - If run without an argument, dumps information about the PC to STDOUT.
 *
 * - If run with a process number created by the current user, 
 *   dumps information about that process to STDOUT.
 *
 * Please change the following before submission:
 *
 * Author: Owen Crewe
 * Login:  ocrewe@uvic.ca 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Note: You are permitted, and even encouraged, to add other
 * support functions in order to reduce duplication of code, or
 * to increase the clarity of your solution, or both.
 */

void print_lines(FILE *fp, int *lines, int lines_len) {
    if (!lines_len)
        return;
    char buffer[256];
    int line = 0;
    int printed = 0;
    while (fgets(buffer, 256, fp) != NULL) {
        for (int i = 0; i<lines_len; ++i) {
            if (lines[i] == line) {
                printf("%s", buffer);
                printed++;
            }
        }
        line++;
    }
}

void print_process_info(char * process_num) {
    
} 


void print_full_info() {
    FILE *cpuinfo;
    FILE *meminfo;
    FILE *version;
    FILE *uptime;
    int lines[2];
    
    cpuinfo = fopen("/proc/cpuinfo", "r"); // model name/cpu cores
    version = fopen("/proc/version", "r"); // linux version
    meminfo = fopen("/proc/meminfo", "r"); // MemTotal
    uptime = fopen("/proc/uptime", "r"); // uptime
    
    // Log model name and cpu cores
    lines[0] = 4;
    lines[1] = 12;
    print_lines(cpuinfo, lines, 2);
    
    // Log linux version
    lines[0] = 0;
    print_lines(version, lines, 1);
    
    // Log MemTotal
    lines[0] = 0;
    print_lines(meminfo, lines, 1);
    
    // Log uptime
    char c = '0';
    char buffer[12];
    int buffer_len = 0;
    while ((c = fgetc(uptime)) != EOF) {
        if (c != '.') {
            buffer[buffer_len++] = c;
        } else {
            buffer[buffer_len++] = '\n';
            buffer[buffer_len++] = '\0';
            break;
        }
    }
    int seconds = atoi(buffer);
    int days = seconds/86400;
    seconds %= 86400;
    int hours = seconds/3600;
    seconds %= 3600;
    int minutes = seconds/60;
    seconds %= 60;
    printf("Updtime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
    
    // Close files
    fclose(cpuinfo);
    fclose(version);
    fclose(meminfo);
    fclose(uptime);
}


int main(int argc, char ** argv) {  
    if (argc == 1) {
        print_full_info();
    } else {
        print_process_info(argv[1]);
    }
}
