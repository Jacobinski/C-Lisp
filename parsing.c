#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>  //TODO: #ifdef _WIN32 doesn't need readline.h to edit lines. Increase portability.

#include "mpc.h"

int main
    (
    int argc,
    char** argv
    )
{
/* Create language parsers */
mpc_parser_t* number     = mpc_new("number");
mpc_parser_t* operator   = mpc_new("operator");
mpc_parser_t* expression = mpc_new("expression");
mpc_parser_t* program    = mpc_new("program");

/* Define the language rules */
mpca_lang(MPCA_LANG_DEFAULT,
    "                                                           \
    number      : /-?[0-9]+/ ;                                  \
    operator    : '+' | '-' | '*' | '/' ;                       \
    expression  : <number> | '(' <operator> <expression>+ ')' ; \
    program     : /^/ <operator> <expression>+ /$/ ;            \
    ",
    number, operator, expression, program);

/* Print out system information */
puts("C Lisp Version 0.1.0");
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

/* Free the parsers */
mpc_cleanup(4, number, operator, expression, program);
}
