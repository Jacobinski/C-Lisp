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
long evaluate
    (
    mpc_ast_t* t
    );

long evaluate_op
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
    number      : /-?[0-9]+/ ;                      \
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
        //mpc_ast_print(r.output);
        long result = evaluate(r.output);
        printf("%li\n", result);
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
long evaluate
    (
    mpc_ast_t* t
    )
{
char* operator;
long number;

/* Base Case: The tree's node is a number */
if( strstr(t->tag, "number") )
    {
    return atoi(t->contents);
    }

/* Recursion: Determine the operator, then use it on all children
 *      Note: '(' is the first child,
 *            the operator is the second child,
 *            numbers are the next tags with the 'expr' tag */
operator = t->children[1]->contents;
number = evaluate(t->children[2]);

for( int ii = 3; strstr(t->children[ii]->tag, "expr"); ++ii )
    {
    number = evaluate_op(number, evaluate(t->children[ii]), operator);
    }

return number;
}
/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
long evaluate_op
    (
    long x,
    long y,
    char* operator
    )
{
if( strcmp(operator, "+") == 0 ) { return x + y; }
if( strcmp(operator, "-") == 0 ) { return x - y; }
if( strcmp(operator, "*") == 0 ) { return x * y; }
if( strcmp(operator, "/") == 0 ) { return x / y; }
if( strcmp(operator, "%") == 0 ) { return x % y; }
return 0;
}
