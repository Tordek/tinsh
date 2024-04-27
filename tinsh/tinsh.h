enum command_line_token_type {
	TOK_EOF,
	TOK_EOL,
	TOK_SPACES,
	TOK_REDIRECT_IN,
	TOK_REDIRECT_OUT,
	TOK_PARAM,
	TOK_PIPE,
	TOK_ENV
};

/**
 * A command line: a series of commands, and possibly redirections.
 */
struct command_line {
	char *stdin;
	char *stdout;
	char **env;
	int env_count;
	int command_count;
	struct command *commands;
};

/**
 * A command: executable filename and parameters.
 */
struct command {
	int length;
	char **content;
};

struct command_line_token {
	enum command_line_token_type type;
	int length;
	int size;
	char *content;
};
