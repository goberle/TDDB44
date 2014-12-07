#include "optimize.hh"

/*** This file contains all code pertaining to AST optimisation. It currently
     implements a simple optimisation called "constant folding". Most of the
     methods in this file are empty, or just relay optimize calls downward
     in the AST. If a more powerful AST optimization scheme were to be
     implemented, only methods in this file should need to be changed. ***/


ast_optimizer *optimizer = new ast_optimizer();


/* The optimizer's interface method. Starts a recursive optimize call down
   the AST nodes, searching for binary operators with constant children. */
void ast_optimizer::do_optimize(ast_stmt_list *body)
{
    if (body != NULL) {
        body->optimize();
    }
}


/* Returns 1 if an AST expression is a subclass of ast_binaryoperation,
   ie, eligible for constant folding. */
bool ast_optimizer::is_binop(ast_expression *node)
{
    switch (node->tag) {
    case AST_ADD:
    case AST_SUB:
    case AST_OR:
    case AST_AND:
    case AST_MULT:
    case AST_DIVIDE:
    case AST_IDIV:
    case AST_MOD:
        return true;
    default:
        return false;
    }
}


/* Returns 1 if an AST expression is a subclass of ast_binaryrelation,
 * ie, eligible for constant folding. */
bool ast_optimizer::is_binrel(ast_expression *node)
{
    switch (node->tag) {
    case AST_EQUAL:
    case AST_NOTEQUAL:
    case AST_GREATERTHAN:
    case AST_LESSTHAN:
        return true;
    default:
        return false;
    }
}


/* We overload this method for the various ast_node subclasses that can
   appear in the AST. By use of virtual (dynamic) methods, we ensure that
   the correct method is invoked even if the pointers in the AST refer to
   one of the abstract classes such as ast_expression or ast_statement. */
void ast_node::optimize()
{
    fatal("Trying to optimize abstract class ast_node.");
}

void ast_statement::optimize()
{
    fatal("Trying to optimize abstract class ast_statement.");
}

void ast_expression::optimize()
{
    fatal("Trying to optimize abstract class ast_expression.");
}

void ast_lvalue::optimize()
{
    fatal("Trying to optimize abstract class ast_lvalue.");
}

void ast_binaryoperation::optimize()
{
    fatal("Trying to optimize abstract class ast_binaryoperation.");
}

void ast_binaryrelation::optimize()
{
    fatal("Trying to optimize abstract class ast_binaryrelation.");
}



/*** The optimize methods for the concrete AST classes. ***/

/* Optimize a statement list. */
void ast_stmt_list::optimize()
{
    if (preceding != NULL) {
        preceding->optimize();
    }
    if (last_stmt != NULL) {
        last_stmt->optimize();
    }
}


/* Optimize a list of expressions. */
void ast_expr_list::optimize()
{
    /* Your code here */
    if (preceding != NULL)
        preceding->optimize();

    if (last_expr != NULL)
        last_expr = optimizer->fold_constants(last_expr);
}


/* Optimize an elsif list. */
void ast_elsif_list::optimize()
{
    /* Your code here */
    if (preceding != NULL)
        preceding->optimize();

    if (last_elsif != NULL)
        last_elsif->optimize();
}


/* An identifier's value can change at run-time, so we can't perform
   constant folding optimization on it unless it is a constant.
   Thus we just do nothing here. It can be treated in the fold_constants()
   method, however. */
void ast_id::optimize()
{
}

void ast_indexed::optimize()
{
    /* Your code here */
    index = optimizer->fold_constants(index);
}

bool ast_optimizer::is_const(ast_expression * expression)
{
    switch (expression->tag)
    {
    case AST_INTEGER:
    case AST_REAL:
        return true;

    case AST_ID:
        return sym_tab->get_symbol_tag(expression->get_ast_id()->sym_p) == SYM_CONST;

    default:
        return false;
    }
}

ast_expression *ast_optimizer::fold_binop(ast_expression *node)
{
    ast_binaryoperation *op = node->get_ast_binaryoperation();

    op->left  = fold_constants(op->left);
    op->right = fold_constants(op->right);

    if (!(is_const(op->left) && is_const(op->right)))
        return node;

    if (op->left->type == integer_type && op->right->type == integer_type)
    {

        long lv = get_value<long>(op->left);
        long rv = get_value<long>(op->right);

        switch (op->tag)
        {
        case AST_ADD:
            return new ast_integer(op->pos, lv + rv);
        case AST_SUB:
            return new ast_integer(op->pos, lv - rv);
        case AST_MULT:
            return new ast_integer(op->pos, lv * rv);
        case AST_AND:
            return new ast_integer(op->pos, lv && rv);
        case AST_OR:
            return new ast_integer(op->pos, lv || rv);
        case AST_IDIV:
            return new ast_integer(op->pos, lv / rv);
        case AST_MOD:
            return new ast_integer(op->pos, lv % rv);
        default:
            return node;
        }
    }
    else if (op->left->type == real_type && op->right->type == real_type)
    {
        double lv = get_value<double>(op->left);
        double rv = get_value<double>(op->right);

        switch (op->tag)
        {
        case AST_ADD:
            return new ast_real(op->pos, lv + rv);
        case AST_SUB:
            return new ast_real(op->pos, lv - rv);
        case AST_MULT:
            return new ast_real(op->pos, lv * rv);
        case AST_DIVIDE:
            return new ast_real(op->pos, lv / rv);
        case AST_AND:
            return new ast_real(op->pos, lv && rv);
        case AST_OR:
            return new ast_real(op->pos, lv || rv);
        default:
            return node;
        }
    }

    return node;
}

ast_expression *ast_optimizer::fold_binrel(ast_expression *node)
{
    ast_binaryrelation *op = node->get_ast_binaryrelation();

    op->left  = fold_constants(op->left);
    op->right = fold_constants(op->right);

    if (!(is_const(op->left) && is_const(op->right)))
      return node;

    if (op->left->type == integer_type && op->right->type == integer_type)
    {
        long lv = get_value<long>(op->left);
        long rv = get_value<long>(op->right);

        switch (op->tag) {
        case AST_EQUAL:
            return new ast_integer(op->pos, lv == rv);
        case AST_NOTEQUAL:
            return new ast_integer(op->pos, lv != rv);
        case AST_GREATERTHAN:
            return new ast_integer(op->pos, lv > rv);
        case AST_LESSTHAN:
            return new ast_integer(op->pos, lv < rv);
        default:
            return node;
        }
    }

    return node;
}


/* This convenience method is used to apply constant folding to all
   binary operations. It returns either the resulting optimized node or the
   original node if no optimization could be performed. */
ast_expression *ast_optimizer::fold_constants(ast_expression *node)
{
    node->optimize();

    if (is_binop(node))
        return optimizer->fold_binop(node);

    if (is_binrel(node))
        return optimizer->fold_binrel(node);

    return node;
}

/* All the binary operations should already have been detected in their parent
   nodes, so we don't need to do anything at all here. */
void ast_add::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_sub::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_mult::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_divide::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_or::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_and::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_idiv::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_mod::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}



/* We can apply constant folding to binary relations as well. */
void ast_equal::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_notequal::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_lessthan::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

void ast_greaterthan::optimize()
{
    /* Your code here */
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}



/*** The various classes derived from ast_statement. ***/

void ast_procedurecall::optimize()
{
    /* Your code here */
    if (parameter_list != NULL)
        parameter_list->optimize();
}


void ast_assign::optimize()
{
    /* Your code here */
    rhs = optimizer->fold_constants(rhs);
}


void ast_while::optimize()
{
    /* Your code here */
    condition = optimizer->fold_constants(condition);
    body->optimize();
}


void ast_if::optimize()
{
    /* Your code here */
    condition = optimizer->fold_constants(condition);
    body->optimize();
    
    if (elsif_list != NULL)
      elsif_list->optimize();

    if (else_body != NULL)
      else_body->optimize();
}


void ast_return::optimize()
{
    /* Your code here */
    value = optimizer->fold_constants(value);
}


void ast_functioncall::optimize()
{
    /* Your code here */
    if (parameter_list != NULL)
        parameter_list->optimize();
}

void ast_uminus::optimize()
{
    /* Your code here */
    expr = optimizer->fold_constants(expr);
}

void ast_not::optimize()
{
    /* Your code here */
    expr = optimizer->fold_constants(expr);
}


void ast_elsif::optimize()
{
    /* Your code here */
    condition = optimizer->fold_constants(condition);
    body->optimize();
}



void ast_integer::optimize()
{
    /* Your code here */
}

void ast_real::optimize()
{
    /* Your code here */
}

/* Note: See the comment in fold_constants() about casts and folding. */
void ast_cast::optimize()
{
    /* Your code here */
    expr = optimizer->fold_constants(expr);
}



void ast_procedurehead::optimize()
{
    fatal("Trying to call ast_procedurehead::optimize()");
}


void ast_functionhead::optimize()
{
    fatal("Trying to call ast_functionhead::optimize()");
}
