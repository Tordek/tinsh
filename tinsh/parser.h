
/**
 * Parse an input line by handling one token at a time.
 *
 * It contains
 */
int parse_commands(struct command_line *command_line);

/**
 * Release memory used when parsing
 */
void free_commands(struct command_line *command_line);