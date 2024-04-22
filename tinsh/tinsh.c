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
    printf("  commands: %d [\n", command_line->command_count);
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
    int(*pipes)[2];
    pipes = malloc(sizeof(int) * 2 * command_line->command_count - 1);

    for (int i = 0; i < command_line->command_count - 1; ++i)
    {
        pipe(pipes[i]);
    }

    for (int i = 0; i < command_line->command_count; ++i)
    {
        int pid = fork();

        if (pid == 0)
        {
            if (i != command_line->command_count - 1)
            {
                close(STDOUT_FILENO);
                dup(pipes[i][1]);
            }

            if (i != 0)
            {
                close(STDIN_FILENO);
                dup(pipes[i - 1][0]);
            }

            for (int i = 0; i < command_line->command_count - 1; ++i)
            {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            execve(command_line->commands[i].content[0],
                   command_line->commands[i].content,
                   command_line->env);
            perror("execve");
            exit(1);
        }
    }

    for (int i = 0; i < command_line->command_count - 1; ++i)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    free(pipes);

    int res;
    wait(&res);
}

int main(int argc, char *argv[])
{
    for (;;)
    {
        struct command_line command_line;
        printf("> ");
        int result = parse_commands(&command_line);

        if (result)
        {
            break;
        }
        run_commands(&command_line);
        free_commands(&command_line);
    }

    return 0;
}