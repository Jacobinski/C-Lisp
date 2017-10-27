#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main
    (
    int argc,
    char** argv
    )
{
puts("C Lisp Version 0.0.0");
puts("Press Ctrl+C to Exit\n");

while(1)
    {
    /*
     * This REPL loop takes a user input and echoes it.
     *
     * TODO: This may need to sleep to reduce CPU load.
     */
    char* input;

    /* Read the user input */
    input = readline("C-Lisp> ");

    /* Allow the user to press up to retrieve command */
    add_history(input);

    /* Echo */
    printf("Echo: %s\n", input);

    /* Free the pointer allocated by readline */
    free(input);
    }
}