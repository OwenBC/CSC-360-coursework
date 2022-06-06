/* gopipe.c
 *
 * CSC 360, Summer 2022
 *
 * Execute up to four instructions, piping the output of each into the
 * input of the next.
 *
 * Please change the following before submission:
 *
 * Author: Owen Crewe
 * Login:  ocrewe@uvic.ca 
 */


/* Note: The following are the **ONLY** header files you are
 * permitted to use for this assignment! */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

#include <stdio.h> // TODO: remember to delete this when done debugging

#define MAX_NUM_COMMANDS 4
#define MAX_INPUT_LINE 80
#define MAX_NUM_TOKENS 8

int tokenize(char *input, char **token_array) {
    int num_tokens;
    char *t;
    
    num_tokens = 0;
    t = strtok(input, " ");
    while (t != NULL) {
        token_array[num_tokens++] = t;
        t = strtok(NULL, " ");
    }
    return num_tokens;
}

int main() {
    // Input
    char *commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS];
    int num_arguments[MAX_NUM_COMMANDS];
    int commands_len = 0;
    char *input;
    char *to_free[MAX_NUM_COMMANDS + 1];
    int to_free_len = 0;
    
    int bytes_read;
    do {
        input = (char *)malloc(MAX_INPUT_LINE*sizeof(char));
        to_free[to_free_len++] = input;
        memset(input, 0, MAX_INPUT_LINE);
        bytes_read = read(1, input, MAX_INPUT_LINE);
        if (bytes_read != 1) {
            if (input[strlen(input) - 1] == '\n')
                input[strlen(input) - 1] = '\0';
            num_arguments[commands_len] = tokenize(input, commands[commands_len]);
            commands_len++;
        }
    } while (commands_len < MAX_NUM_COMMANDS && bytes_read != 1);
    
    for (int i = 0; i<commands_len; i++) {
        for (int j = 0; j<num_arguments[i]; j++) {
            printf("%s", commands[i][j]);
        }
        printf("\n");
    }
    
    // Setup piping
    
    // Execute
    for (int i = 0; i<commands_len; i++)
        free(to_free[i]);
}
