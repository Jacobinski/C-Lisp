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
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SEXPR,
    LVAL_SYM
    };

typedef struct
    {
    lval_type_field type;
    char* err;
    char* sym;
    long num;
    int cell_count;
    struct lval** cell;
    } lval;

/*---------------------------------------------------------------------
 * FUNCTION DECLARATIONS
 *---------------------------------------------------------------------*/
/* Evaluators */
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

lval* lval_read
    (
    mpc_ast_t* t
    );

lval* lval_read_num
    (
    mpc_ast_t* t
    );

lval* lval_add
    (
    mpc_ast_t* v,
    mpc_ast_t* x
    );

/* Constructors */
lval* lval_num
    (
    long num
    );

lval* lval_err
    (
    char* msg
    );

lval* lval_sym
    (
    char* sym
    );

lval* lval_sexpr
    (
    void
    );

void lval_del
    (
    lval* v
    );

/* Utility */
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
mpc_parser_t* number      = mpc_new("number");
mpc_parser_t* symbol      = mpc_new("symbol");
mpc_parser_t* sexpression = mpc_new("sexpression");
mpc_parser_t* expression  = mpc_new("expression");
mpc_parser_t* program     = mpc_new("program");

/* Define the language rules */
mpca_lang(MPCA_LANG_DEFAULT,
    "                                                   \
    number      : /-?[0-9]+/ ;                          \
    symbol      : '+' | '-' | '*' | '/' | '%' ;         \
    sexpression : '(' <expression>* ')' ;               \
    expression  : <number> | <symbol> | <sexpression> ; \
    program     : /^/ <expression>* /$/ ;               \
    ",
    number, symbol, sexpression, expression, program);

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
mpc_cleanup(5, number, symbol, sexpression, expression, program);
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
lval* lval_read
    (
    mpc_ast_t* t
    )
{
/* If Symbol or Number return conversion to that type */
if( strstr(t->tag, "number") ) { return lval_read_num(t); }
if( strstr(t->tag, "symbol") ) { return lval_sym(t->contents); }

/* If root (>) or sexpr then create empty list */
lval* x = NULL;
if( strcmp(t->tag, ">") == 0 ) { x = lval_sexpr(); }
if( strstr(t->tag, "sexpr") )  { x = lval_sexpr(); }

/* Fill empty list with valid expressions from children */
for( int i = 0; i < t->childen_num; ++i )
    {
    // TODO: Why do we ignore brackets?
    if( strcmp(t->children[i]->contents, "(") == 0 ) { continue; }
    if( strcmp(t->children[i]->contents, ")") == 0 ) { continue; }
    if( strcmp(t->children[i]->tag,  "regex") == 0 ) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
    }

return x;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval* lval_read_num
    (
    mpc_ast_t* t
    )
{
/* Convert str->long, then check stdlib's errno for overflow */
long num;

errno = 0;
num = strtol(t->contents, NULL, 10);
return errno != ERANGE
    ? lval_num(num)
    : lval_err("Invalid number");
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval* lval_add
    (
    mpc_ast_t* v,
    mpc_ast_t* x
    )
{
v->cell_count++;
v->cell = realloc(v->cell, sizeof(lval*) * v->cell_count);
v->cell[v->cell_count - 1] = x;
return v;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval* lval_num
    (
    long num
    )
{
lval* lisp_value;

lisp_value = malloc(sizeof(lval));
lisp_value->type = LVAL_NUM;
lisp_value->num = num;

return lisp_value;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval* lval_err
    (
    char* msg
    )
{
lval* lisp_value;

lisp_value = malloc(sizeof(lval));
lisp_value->type = LVAL_ERR;
lisp_value->err = malloc(strlen(msg) + 1);
strcpy(lisp_value->err, msg)

return lisp_value;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval* lval_sym
    (
    char* sym
    )
{
lval* lisp_value;

lisp_value = malloc(sizeof(lval));
lisp_value->type = LVAL_SYM;
lisp_value->sym = malloc(strlen(sym) + 1);
strcpy(lisp_value->sym, sym)

return lisp_value;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
lval* lval_sexpr
    (
    void
    )
{
lval* lisp_value;

lisp_value = malloc(sizeof(lval));
lisp_value->type = LVAL_SEXPR;
lisp_value->cell = NULL;
lisp_value->cell_count = 0;

return lisp_value;
}

/*---------------------------------------------------------------------
 *---------------------------------------------------------------------*/
void lval_del
    (
    lval* v
    )
{
switch(v->type)
    {
    /* Do nothing special for Num */
    case LVAL_NUM:
        break;

    /* Free string data for Err and Sym */
    case LVAL_ERR: free(v->err):
        break;
    case LVAL_SYM: free(v->sym):
        break;

    /* Free all elements in Sexpr */
    case LVAL_SEXPR:
        for( int i = 0; i < v->cell_count; ++i )
            {
            lval_del(v->cell[i]);
            }
        /* Free the array of pointers */
        free(v->cell);
        break;
    }

free(v);
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
