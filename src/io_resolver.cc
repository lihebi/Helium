#include "resolver.h"

using namespace ast;


/**
 * resolve the type of the variable "name", at the Node n in AST.
 
 This is fairly easy: from the given node, go to previous sibling and parent.
There're two kinds of stmts that defines a variable:
- funtion parameter
- decl_stmt

Bottom Up
 
 */
Variable resolver::resolve_var(ast::Node node, const std::string& name) {
  if (!node) return Variable(); // node is empty, return empty Variable
  switch (node.Kind()) {
  case NK_Function: {
    // examine function parameters
    VariableList vars = var_from_node(node);
    if (vars.LookUp(name)) return vars.LookUp(name);
    break;
  }
  case NK_DeclStmt: {
    // examine decl_stmt
    VariableList vars = var_from_node(node);
    if (vars.LookUp(name)) return vars.LookUp(name);
    break;
  }
  default: {
    // go to next sibling or parent to recurse.
    if (node.NextSibling()) return resolve_var(node.NextSibling(), name);
    if (node.Parent()) return resolve_var(node.Parent(), name);
    // if no sibling and parent, return Null variable
    return Variable();
  }
  }
  return Variable();
}


/**
 * Get a list of alive variables ** at Node "node"**.
 This does not include undefined variable(we only search the variable defines)

From the node, go to previous sibling and parent.
There're two kinds of stmts that defines a variable:
- function parameter
- decl_stmt

The extra restriction is, the nodes to search must be inside the node list.

This is somehow a symbol table. We need to use the inner most declaration for that variable.

Bottom Up
 */

void resolver::get_alive_vars(ast::Node node, ast::NodeList nodes, VariableList &result) {
  if (!node) return;
  // if (!nodes.Contains(node)) return; // must be inside node list given. It is not the node in the list, but the node is inside some node in the list.
  if (ast::contains(nodes, node)) return;
  switch (node.Kind()) {
  case NK_Function:
  case NK_DeclStmt: {
    VariableList vars = var_from_node(node);
    result.AddUniqueName(vars);
    break;
  }
  default: {
    if (node.NextSibling()) return get_alive_vars(node.NextSibling(), nodes, result);
    if (node.Parent()) return get_alive_vars(node.Parent(), nodes, result);
    return;
  }
  }
}

void resolver::get_undefined_vars(ast::NodeList nodes, VariableList &result) {
  SymbolTable st;
  get_undefined_vars(nodes, st, result);
}

/**

 * Get a list of variables that is used in the node list, but not defined within them.
 
 Top Down.

All variable usage is parsed as <name> tag in <expr> tag.
But, it is not marked as variable name, or function name, or other names.

We need to build the symbol table along the way.
Currently symbol table only contains variable.
Variables can be defined in
- for init
- decl_stmt

 from the first node in the node list, depth first search, maintain symbol table.
 
 @param[in] nodes the nodes to search in this level
@param[in] the symbol table until this level
@param[in,out] the variable list that is undefined
 */
void resolver::get_undefined_vars(ast::NodeList nodes, SymbolTable &st, VariableList &result) {
  for (ast::Node node : nodes) {
    switch (node.Kind()) {
    case NK_DeclStmt: {
      VariableList vars = var_from_node(node);
      st.AddSymbol(vars);
      break;
    }
    case NK_For: {
      VariableList vars = var_from_node(node);
      st.PushLevel();
      st.AddSymbol(vars);
      get_undefined_vars(node.Children(), st, result);
      st.PopLevel();
      break;
    }
    case NK_Expr: {
      
      std::vector<std::string> names = dynamic_cast<ExprNode&>(node).IDs();//  = parse_expr(node); // TODO replace it with semantic one
      for (auto it=names.begin();it!=names.end();++it) {
        if (st.LookUp(*it)) continue;
        Variable v = resolve_var(node, *it);
        // also add unique name, because I can't think up a scenario that an inner variable with the same name is not defined
        // and there's no way to init them anyhow.
        // The only problem that will arise is that the block is not continuous
        if (v) result.AddUniqueName(v);
      }
    }
    default: {
      st.PushLevel();
      get_undefined_vars(node.Children(), st, result);
      st.PopLevel();
    }
    }
  }
}

