Let's build a shell.

# Scope

No scripting, just running commands.

It must take one line, parse it, and run the command, printing out the result.

It must handle pipes.

It must handle <, >.

# Syntax

Each line contains one or more commands separated by pipes.

Each command contains one executable file and any number of parameters.

Parameters can be quoted or escaped.

There can be up to one "< file" redirection and up to one "> file" redirection. They consume the file. 