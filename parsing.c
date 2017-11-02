/*---------------------------------------------------------------------
 * HEADERS
 *---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>  //TODO: #ifdef _WIN32 doesn't need readline.h to edit lines. Increase portability.

#include "mpc.h"

/*---------------------------------------------------------------------
 * TYPE DECLARATIONS
 *---------------------------------------------------------------------*/
typedef int lval_type_field; enum
    {
    LVAL_NUM,
    LVAL_ERR
    };

typedef int lval_err_field; enum
    {
    LVAL_ERR_DIV_ZERO,
    LVAL_ERR_BAD_OP,
    LVAL_ERR_BAD_NUM,

    LVAL_ERR_NONE
    };

typedef struct
    {
    lval_type_field type;
    lval_err_field err;
    long num;
    } lval;

/*---------------------------------------------------------------------
 * FUNCTION DECLARATIONS
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

lval lval_create_num
    (
    long num
    );

lval lval_create_err
    (
    lval_err_field err
    );

void lval_print
    (
    lval val
    );

void lval_println
    (
    lval val
    );

/*---------------------------------------------------------------------
 * FUNCTIONS
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
        /* Success: Evaluate the expression */
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

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval lval_create_num
    (
    long num
    )
{
lval lisp_value;

lisp_value.type = LVAL_NUM;
lisp_value.num = num;
lisp_value.err = LVAL_ERR_NONE;

return lisp_value;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval lval_create_err
    (
    lval_err_field err
    )
{
lval lisp_value;

lisp_value.type = LVAL_ERR;
lisp_value.num = 0;
lisp_value.err = err;

return lisp_value;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
void lval_print
    (
    lval val
    )
{
switch( val.type )
    {
    case LVAL_NUM:
        printf("%li\n", val.num);
        break;

    case LVAL_ERR:
        if( val.err == LVAL_ERR_DIV_ZERO )
            printf("Error: Division by zero.");
        else if( val.err == LVAL_ERR_BAD_OP )
            printf("Error: Invalid operation.");
        else if( val.err == LVAL_ERR_BAD_NUM )
            printf("Error: Invalid number.");
        else
            printf("Error: Unknown Error.");
        break;

    default:
        printf("Error: Unable to evaluate lval type.");
        break;
    }
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
void lval_println
    (
    lval val
    )
{
lval_print(val);
putchar('\n');
};
