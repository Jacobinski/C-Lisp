/*---------------------------------------------------------------------
 *                               HEADERS
 *---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>  //TODO: #ifdef _WIN32 doesn't need readline.h to edit lines. Increase portability.

#include "mpc.h"

/*---------------------------------------------------------------------
 *                          FUNCTION DECLARATIONS
 *---------------------------------------------------------------------*/
long evaluate_tree
    (
    mpc_ast_t* tree
    );

long evaluate_operator
    (
    long x,
    long y,
    char* operator
    );

/*---------------------------------------------------------------------
 *                              FUNCTIONS
 *---------------------------------------------------------------------*/
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
    number      : /-?[0-9]+(\\.[0-9]+)?/ ;                      \
    operator    : '+' | '-' | '*' | '/' | '%' ;                 \
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
     * This REPL loop takes a user input and parses it.
     */
    char* input;
    mpc_result_t r;

    /* Read the user input */
    input = readline("C-Lisp> ");

    /* Allow the user to press up to retrieve command */
    add_history(input);

    /* Parse the user input */
    if( mpc_parse("<stdin>", input, program, &r) )
        {
        /* Success: Print the abstract syntax tree */
        mpc_ast_print(r.output);
        mpc_ast_delete(r.output);
        }
    else
        {
        /* Failure: Print the error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
        }

    /* Free the pointer allocated by readline */
    free(input);
    }

/* Free the parsers */
mpc_cleanup(4, number, operator, expression, program);
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
long evaluate_tree
    (
    mpc_ast_t* tree
    )
{
return 0;
}
/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
long evaluate_operator
    (
    long x,
    long y,
    char* operator
    )
{
return 0;
}
