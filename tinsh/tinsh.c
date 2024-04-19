#include <stdio.h>
#include <stdlib.h>
#include "tinsh.h"

#define MAXLINE 256

/**
 * Read up to one line from STDIN and call it a day.
 */
void parse_commands(struct command_line *command_line)
{
    command_line->stdin = NULL;
    command_line->stdout = NULL;
    command_line->command_count = 0;
    command_line->commands = NULL;
    char *result;
    do
    {
        // Optimistically allocate for one more command.
        command_line->commands = realloc(command_line->commands,
                                         sizeof(struct command) * (command_line->command_count + 1));

        command_line->commands[command_line->command_count].argv = NULL;
        command_line->commands[command_line->command_count].argc = 0;
        command_line->commands[command_line->command_count].argv = realloc(
            command_line->commands[command_line->command_count].argv,
            sizeof(char *) * (command_line->commands[command_line->command_count].argc + 1));
        command_line->commands[command_line->command_count].argv[
            command_line->commands[command_line->command_count].argc] = malloc(MAXLINE);        
        result = fgets(command_line->commands[command_line->command_count].argv[command_line->commands[command_line->command_count].argc], MAXLINE, stdin);
        command_line->commands[command_line->command_count].argc += 1;
        command_line->command_count++;
    } while (0); // Just read once for now.
}

/**
 * Release memory used when parsing
 */
void free_commands(struct command_line *command_line)
{
    free(command_line->stdin);
    free(command_line->stdout);

    for (int i = 0; i < command_line->command_count; ++i)
    {
        free(command_line->commands[i].argv);
    }

    free(command_line->commands);
}

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
        printf("    ");
        for (int j = 0; j < command_line->commands[i].argc; ++i)
        {
            printf("%s", command_line->commands[i].argv[j]);
        }
    }
    printf("  ]\n");
    printf("}\n");
}

int main(int argc, char *argv[])
{
    struct command_line command_line;

    parse_commands(&command_line);
    print_commands(&command_line);
    free_commands(&command_line);

    return 0;
}