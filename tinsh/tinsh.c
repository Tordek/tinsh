#include <stdio.h>
#include <stdlib.h>
#include "tinsh.h"

#define MAXLINE 256

static int parse_token(struct command_line_token *token);

static void append_to_token(struct command_line_token *token, char c)
{
    if (token->length == token->size)
    {
        token->content = realloc(token->content, token->size * 2);
        token->size *= 2;
    }
    token->content[token->length] = c;
    token->length++;
}

static void end_token(struct command_line_token *token)
{
    if (token->length == token->size)
    {
        token->content = realloc(token->content, token->size + 1);
        token->size += 1;
    }
    token->content[token->length] = '\0';
}

static void append_to_command(struct command *command, char *token)
{
    command->length++;
    command->content = realloc(command->content, sizeof(char *) * command->length);
    command->content[command->length - 1] = token;
}

/**
 * Consume a series of spaces.
 */
static int parse_spaces(struct command_line_token *token)
{
    token->size = 1;
    token->length = 0;
    token->content = malloc(1);
    token->type = SPACES;

    for (;;)
    {
        int c = fgetc(stdin);

        if (c == EOF || c == '\n')
        {
            return 1;
        }

        if (c != ' ')
        {
            ungetc(c, stdin);
            end_token(token);
            return 1;
        }

        append_to_token(token, c);
    }
}

static int is_valid_in_identifier(int c)
{
    return !(c == EOF || c == '\n' || c == ' ' || c == '<' || c == '>' || c == '|');
}

static int parse_redirect(struct command_line_token *token)
{
    int direction = fgetc(stdin);

    // Find the target: discard spaces and retry
    do
    {
        int result = parse_token(token);

        if (result == -1)
        {
            return -1;
        }

        if (token->type == SPACES)
        {
            free(token->content);
            continue;
        }

        if (token->type != PARAM)
        {
            return -1;
        }
    } while (0);

    token->type = direction == '<' ? REDIRECT_IN : REDIRECT_OUT;
    return 1;
}

static int parse_pipe(struct command_line_token *token)
{
    fgetc(stdin); // discard
    token->type = PIPE;
    token->size = 0;
    token->length = 0;
    token->content = NULL;
    return 1;
}

static int parse_quoted(struct command_line_token *token)
{
    for (;;)
    {
        int c = fgetc(stdin);

        if (c == EOF)
        {
            return -1;
        }

        if (c == '"')
        {
            return;
        }

        append_to_token(token, c);
    }
}

static int parse_param(struct command_line_token *token)
{
    token->size = 32;
    token->length = 0;
    token->content = malloc(32);
    token->type = PARAM;

    for (;;)
    {
        int c = fgetc(stdin);

        if (!is_valid_in_identifier(c))
        {
            ungetc(c, stdin);
            break;
        }

        if (c == '"')
        {
            int result = parse_quoted(token);
            if (result == -1)
            {
                return -1;
            }
            continue;
        }

        append_to_token(token, c);
    }

    end_token(token);
    return 1;
}

/**
 * Read a single token
 *
 * Returns:
 * -1 on error (premature eof)
 * 0 when there is nothing to read.
 * 1 when reading a token correctly.
 */
static int parse_token(struct command_line_token *token)
{
    int c = fgetc(stdin);

    // End parsing
    if (c == '\n' || c == EOF)
    {
        return 0;
    }

    ungetc(c, stdin);
    switch (c)
    {
    case ' ':
        return parse_spaces(token);

    case '<':
    case '>':
        return parse_redirect(token);

    case '|':
        return parse_pipe(token);

    default:
        return parse_param(token);
    }
}

static int parse_command(struct command_line *command_line)
{
    command_line->command_count++;
    command_line->commands = realloc(command_line->commands, sizeof(struct command) * command_line->command_count);
    struct command *command = &command_line->commands[command_line->command_count - 1];

    for (;;)
    {
        struct command_line_token token;
        int result = parse_token(&token);
        if (result == 0)
        {
            return 0;
        }

        switch (token.type)
        {
        // Spaces do nothing.
        case SPACES:
            free(token.content);
            continue;

        // On redirect, remember where to.
        // TODO: Allow multiple output redirects
        case REDIRECT_IN:
            command_line->stdin = token.content;
            break;

        case REDIRECT_OUT:
            command_line->stdin = token.content;
            break;

        // On pipe, start a new command.
        case PIPE:
            return 1;
            break;

        // Any other param just gets put in the list.
        case PARAM:
            append_to_command(command, token.content);
            break;

        default:
            break;
        }
    }
}

/**
 * Parse an input line by handling one token at a time.
 *
 * It contains
 */
void parse_commands(struct command_line *command_line)
{
    command_line->stdin = NULL;
    command_line->stdout = NULL;
    command_line->command_count = 0;
    command_line->commands = NULL;

    while (parse_command(command_line))
        ;
}

// /**
//  * Release memory used when parsing
//  */
// void free_commands(struct command_line *command_line)
// {
// }

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
        for (int j = 0; j < command_line->commands[i].length; ++j)
        {
            printf("%s ", command_line->commands[i].content[j]);
        }
        printf("\n");
    }
    printf("  ]\n");
    printf("}\n");
}

int main(int argc, char *argv[])
{
    struct command_line command_line;
    parse_commands(&command_line);
    print_commands(&command_line);
    // free_commands(&command_line);

    return 0;
}