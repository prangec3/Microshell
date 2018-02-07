# Microshell

Built on Ubuntu

Compiling: gcc -o ush ush.c builtin.c arg_parse.c

Usage: ./ush

Built-In Functions:

exit: exits with the specified exit code (e.g. exit 1), or 0 if none is provided.
aecho: prints all arguments. If the second argument is -n, does not print a newline character.
cd: change directory to specified directory (e.g. cd ${HOME}).
envset: set an environmental variable (e.g. envset one two) -> (aecho ${one}) -} (two)
envunset: unset an environmental variable (evunset one)
