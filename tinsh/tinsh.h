/**
 * A command: executable filename and parameters.
 */
struct command
{
    char *executable;
    char **params;
};

/**
 * A command line: a series of commands, and possibly redirections. 
 */
struct command_line
{
    char *stdin;
    char *stdout;
    int command_count;
    struct command *commands;
};