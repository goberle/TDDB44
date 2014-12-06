#ifndef __OPTIMIZE_HH__
#define __OPTIMIZE_HH__

#include "ast.hh"


/*** This class performs AST optimisation. Currently it only implements a
     very simple optimisation known as constant folding, which means that it
     tries to evaluate a binary operation node such as 2 + 5 during compiling,
     replacing it with a single integer node with value 7, or an expression
     only involving constants, such as (assuming FOO = 2) 4 + FOO, replacing
     the + node with an integer node with the value 6. ***/


class ast_optimizer;

// Defined in optimize.cc.
extern ast_optimizer *optimizer;


class ast_optimizer
{
/* You might want to add your own methods to this header file when
   solving the optimization lab. */
    
public:

    // This is the interface to parser.y. Sending in a function body as
    // arguments performs (destructive) optimization on it.
    void do_optimize(ast_stmt_list *);

    // Returns if the argument is a subclass of ast_binaryoperation.
    // It's needed to find out which nodes are eligible for optimization.
    bool is_binop(ast_expression *);

    // Return if the argument is to be considered as constant
    bool is_const(ast_expression *);

    template <typename T>
    T get_value(ast_expression * node)
    {
        switch (node->tag)
        {
        case AST_INTEGER:
            return node->get_ast_integer()->value;

        case AST_REAL:
            return node->get_ast_real()->value;

        case AST_ID:
            return *((T *)&sym_tab->get_symbol(node->get_ast_id()->sym_p)->get_constant_symbol()->const_value);

        default:
            fatal("not possible to get a value");
            return 0;
        }
    }

    // This is a convenient method used in optimize.cc. It has to be public
    // so the ast_* nodes can access it. Another solution would be to make it
    // a static method in the optimize.cc file... A matter of preference.
    ast_expression *fold_constants(ast_expression *);
};


#endif
