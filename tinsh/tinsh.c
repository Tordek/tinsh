#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tinsh.h"
#include "parser.h"
#include <sys/types.h>
#include <sys/wait.h> 

/**
 * Debugging: print out the command line
 */
void print_commands(struct command_line *command_line)
{
    printf("{\n");
    printf("  stdin: %s\n", command_line->stdin);
    printf("  stdout: %s\n", command_line->stdout);
    printf("  commands: [\n");
    for (int i = 0; i < command_line->command_count; ++i)
    {
        printf("   %d ", i);
        for (int j = 0; j < command_line->commands[i].length; ++j)
        {
            printf("%s ", command_line->commands[i].content[j]);
        }
        printf("\n");
    }
    printf("  ]\n");
    printf("}\n");
}

/**
 * Debugging: print out the command line
 */
void run_commands(struct command_line *command_line)
{
    for (int i = 0; i < command_line->command_count; ++i)
    {
        int pid = fork();

        if (pid == 0)
        {
            execv(command_line->commands[i].content[0],
                  command_line->commands[i].content);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
    }
}

int main(int argc, char *argv[])
{
    for (;;)
    {
        struct command_line command_line;
        printf("> ");
        parse_commands(&command_line);
        // print_commands(&command_line);
        run_commands(&command_line);
        free_commands(&command_line);
    }

    return 0;
}