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
lval evaluate
    (
    mpc_ast_t* t
    );

lval evaluate_op
    (
    lval x,
    lval y,
    char* operator
    );

lval lval_num
    (
    long num
    );

lval lval_err
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
puts("C Lisp Version 0.0.0");
puts("Press Ctrl+C to Exit\n");

while(1)
    {
    /* This REPL loop takes a user input and parses it */
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
        lval result = evaluate(r.output);
        lval_println(result);
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
lval evaluate
    (
    mpc_ast_t* t
    )
{
char* operator;
lval val;

/* Base Case: The tree's node is a number */
if( strstr(t->tag, "number") )
    {
    /* Convert str->long, then check stdlib's errno for overflow */
    long num;

    errno = 0;
    num = strtol(t->contents, NULL, 10);
    return errno != ERANGE
        ? lval_num(num)
        : lval_err(LVAL_ERR_BAD_NUM);
    }

/* Recursion: Determine the operator, then use it on all children.
 *            1st child: '('
 *            2nd child: an operator
 *            Nth child: Numbers/expressions. These are ended by the non-expr ')' */
operator = t->children[1]->contents;
val = evaluate(t->children[2]);

for( int ii = 3; strstr(t->children[ii]->tag, "expr"); ++ii )
    {
    val = evaluate_op(val, evaluate(t->children[ii]), operator);
    }

return val;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval evaluate_op
    (
    lval x,
    lval y,
    char* operator
    )
{
if( LVAL_ERR == x.type ) { return x; }
if( LVAL_ERR == y.type ) { return y; }

// FIXME: Error check for overlow in the float values
if( strcmp(operator, "+") == 0 ) { return lval_num(x.num + y.num); }
if( strcmp(operator, "-") == 0 ) { return lval_num(x.num - y.num); }
if( strcmp(operator, "*") == 0 ) { return lval_num(x.num * y.num); }
if( strcmp(operator, "%") == 0 ) { return lval_num(x.num % y.num); }
if( strcmp(operator, "/") == 0 )
    {
    return y.num == 0
        ? lval_err(LVAL_ERR_DIV_ZERO)
        : lval_num(x.num / y.num);
    }

return lval_err(LVAL_ERR_BAD_OP);
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval lval_num
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
lval lval_err
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
        printf("%li", val.num);
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
