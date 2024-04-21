enum command_line_token_type
{
    NONE,
    SPACES,
    REDIRECT_IN,
    REDIRECT_OUT,
    PARAM,
    PIPE,
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

/**
 * A command: executable filename and parameters.
 */
struct command
{
    int length;
    char **content;
};

struct command_line_token
{
    enum command_line_token_type type;
    int length;
    int size;
    char *content;
};


