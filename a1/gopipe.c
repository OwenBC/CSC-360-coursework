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

#define MAX_NUM_COMMANDS 4
#define MAX_INPUT_LINE 80
#define MAX_NUM_TOKENS 8

// max int function https://codeforwin.org/2016/02/c-program-to-find-maximum-and-minimum-using-functions.html
int max(int num1, int num2) {
    return (num1 > num2 ) ? num1 : num2;
}

void tokenize(char *input, char **token_array) {
    int num_tokens;
    char *t;
    
    num_tokens = 0;
    t = strtok(input, " ");
    while (t != NULL) {
        token_array[num_tokens++] = t;
        t = strtok(NULL, " ");
    }
    token_array[num_tokens] = 0;
}

int main() {
    // Input
    char *commands[MAX_NUM_COMMANDS][MAX_NUM_TOKENS+1];
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
            tokenize(input, commands[commands_len]);
            commands_len++;
        }
    } while (commands_len < MAX_NUM_COMMANDS && bytes_read != 1);
    
    // Setup piping
    char *envp[] = { 0 };
    int pid[commands_len];
    int fd[max(0, commands_len-1)][2];
    int status;
    
    for (int i = 0; i<commands_len-1; i++)
        pipe(fd[i]);
    
    for (int i = 0; i<commands_len; i++) {
        if ((pid[i] = fork()) == 0) {
            for (int j = 0; j<commands_len-1; j++) {
                // Read end, connect if j = i-1
                if (j == i - 1) {
                    dup2(fd[j][0], 0);
                }else{
                    close(fd[j][0]);
                }
                // Write end, connect if j = i
                if (j == i){
                    dup2(fd[j][1], 1);
                }else{
                    close(fd[j][1]);
                }
            }
            execve(commands[i][0], commands[i], envp);
        }
    }
    
    // Close pipes in parent
    for (int i = 0; i<commands_len-1; i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
    
    // Wait for children
    for (int i = 0; i<commands_len; i++)
        waitpid(pid[i], &status, 0);
    
    // Free memory
    for (int i = 0; i<to_free_len; i++)
        free(to_free[i]);
}
