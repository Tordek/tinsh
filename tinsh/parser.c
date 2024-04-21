
#include <stdio.h>
#include <stdlib.h>
#include "tinsh.h"
#include "parser.h"

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
  command->content[command->length] = NULL;
}

static int is_eol(int c)
{
  return c == EOF || c == '\n' || c == ';';
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

    if (is_eol(c))
    {
      return 0;
    }

    if (c != ' ')
    {
      ungetc(c, stdin);
      end_token(token);
      return 0;
    }

    append_to_token(token, c);
  }
}

/**
 * Parses a > or <, and handles what comes next: possibly a space, followed by
 * the target.
 */
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
  return 0;
}

/**
 * Parses a |
 */
static int parse_pipe(struct command_line_token *token)
{
  fgetc(stdin); // discard
  token->type = PIPE;
  token->size = 0;
  token->length = 0;
  token->content = NULL;
  return 0;
}

/**
 * Valid characters outside quotes
 */
static int is_valid_in_identifier(int c)
{
  return !(is_eol(c) || c == ' ' || c == '<' || c == '>' || c == '|' || c == '"');
}

/**
 * Helper for parse_param: Stuff inside quotes can contain special characters.
 */
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
      return 0;
    }

    append_to_token(token, c);
  }
}

/**
 * Parses a param, ends on any character that can't be part of a param.
 */
static int parse_param(struct command_line_token *token)
{
  token->size = 32;
  token->length = 0;
  token->content = malloc(32);
  token->type = PARAM;

  for (;;)
  {
    int c = fgetc(stdin);

    if (c == '"')
    {
      int result = parse_quoted(token);
      if (result == -1)
      {
        return -1;
      }
      continue;
    }

    if (!is_valid_in_identifier(c))
    {
      ungetc(c, stdin);
      end_token(token);
      return 0;
    }

    append_to_token(token, c);
  }
}

/**
 * Parses whatever can end a line: \n, EOF, ;.
 */
static int parse_eol(struct command_line_token *token)
{
  token->type = EOL;
  token->size = 0;
  token->length = 0;
  token->content = NULL;
  fgetc(stdin); // discard

  return 0;
}

/**
 * Read a single token
 *
 * Returns:
 * -1 on error (premature eof)
 * 0 when reading a token correctly.
 */
static int parse_token(struct command_line_token *token)
{
  int c = fgetc(stdin);
  ungetc(c, stdin);
  switch (c)
  {
  case EOF:
  case '\n':
  case ';':
    return parse_eol(token);

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
  for (;;)
  {
    struct command_line_token token;
    int result = parse_token(&token);
    if (result == -1)
    {
      return -1;
    }
    switch (token.type)
    {
    case EOL:
      if (command_line->command_count == 0)
      {
        return -1;
      }
      return 0;

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
      append_to_command(&command_line->commands[command_line->command_count - 1], token.content);
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
int parse_commands(struct command_line *command_line)
{
  command_line->stdin = NULL;
  command_line->stdout = NULL;
  command_line->command_count = 1;
  command_line->commands = malloc(sizeof(struct command));

  for (;;)
  {
    int result = parse_command(command_line);
    if (result == -1)
    {
      return -1;
    }

    if (result == 0)
    {
      return 0;
    }
    command_line->command_count++;
    command_line->commands = realloc(command_line->commands, sizeof(struct command) * command_line->command_count);
  }
}

/**
 * Release memory used when parsing
 */
void free_commands(struct command_line *command_line)
{
  for (int i = 0; i < command_line->command_count; ++i)
  {
    for (int j = 0; j < command_line->commands[i].length; ++j)
    {
      free(command_line->commands[i].content[j]);
    }
    free(command_line->commands);
  }
}