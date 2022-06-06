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

void print_line(char *path, int line, char *prefix) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Error: PID not found.\n");
        exit(1);
    }
    char buffer[256];
    int line_counter = 0;
    while (fgets(buffer, 256, fp) != NULL && line_counter < line)
        line_counter++;
    fclose(fp);
    printf("%s%s", prefix, buffer);
}

int read_next_int_from_stream(FILE* fp) {
    char c = '0';
    char buffer[256];
    int buffer_len = 0;
    char number_found = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (c > 47 && c < 58) {
            buffer[buffer_len++] = c;
            number_found = 1;
        } else if (number_found) {
            buffer[buffer_len++] = '\n';
            buffer[buffer_len++] = '\0';
            break;
        }
    }
    return atoi(buffer);
}

void print_process_info(char * process_num) {
    printf("Process number: %s\n", process_num);
    
    char path[256] = "/proc/";
    int offset = 6;
    strcpy(path+offset, process_num);
    offset += strlen(process_num);
    
    // Log process name
    strcpy(path+offset, "/status");
    print_line(path, 0, "");
    
    // Log process "Filename"
    strcpy(path+offset, "/comm");
    print_line(path, 0, "Filename (if any): ");
    
    // Log # of threads
    strcpy(path+offset, "/status");
    print_line(path, 33, "");
    
    // Log total context switches    
    FILE *uptime = fopen(path, "r"); // uptime
    char buffer[256];
    int line_counter = 0;
    while (fgets(buffer, 256, uptime) != NULL && line_counter < 52)
        line_counter++;
    int total = read_next_int_from_stream(uptime) + read_next_int_from_stream(uptime);
    fclose(uptime);
    
    printf("Total context switches: %d\n", total);
    
    
} 


void print_full_info() {
    // Log model name and cpu cores
    print_line("/proc/cpuinfo", 4, "");
    print_line("/proc/cpuinfo", 12, "");
    
    // Log linux version
    print_line("/proc/version", 0, "");
    
    // Log MemTotal
    print_line("/proc/meminfo", 0, "");
    
    // Log uptime
    FILE *uptime = fopen("/proc/uptime", "r"); // uptime
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
    fclose(uptime);
    int seconds = atoi(buffer);
    
    int days = seconds/86400;
    seconds %= 86400;
    int hours = seconds/3600;
    seconds %= 3600;
    int minutes = seconds/60;
    seconds %= 60;
    
    printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
}


int main(int argc, char ** argv) {  
    if (argc == 1) {
        print_full_info();
    } else {
        print_process_info(argv[1]);
    }
}
