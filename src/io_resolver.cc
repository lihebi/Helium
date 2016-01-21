#include "resolver.h"
#include "utils.h"

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
  switch (kind(node)) {
  case NK_Function: {
    // examine function parameters
    VariableList vars = var_from_node(node);
    // for (Variable v : vars) {
    //   std::cout <<v.Name() << ":" << v.GetType().ToString()  << "\n";
    // }
    if (look_up(vars, name)) {
      return look_up(vars, name);
    }
    break;
  }
  case NK_DeclStmt: {
    // examine decl_stmt
    VariableList vars = var_from_node(node);
    // for (Variable v : vars) {
    //   std::cout <<v.Name() << ":" << v.GetType().ToString()  << "\n";
    // }
    if (look_up(vars, name)) {
      return look_up(vars, name);
    }
    break;
  }
  default: {
    // do nothing
  }
  }
  // go to next sibling or parent to recurse.
  if (previous_sibling(node)) return resolve_var(previous_sibling(node), name);
  if (parent(node)) return resolve_var(parent(node), name);
  // if no sibling and parent, return Null variable
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
  switch (kind(node)) {
  case NK_Function:
  case NK_DeclStmt: {
    VariableList vars = var_from_node(node);
    for (Variable v : vars) {
      add_unique(result, v);
    }
    break;
  }
  default: {
    if (next_sibling(node)) return get_alive_vars(next_sibling(node), nodes, result);
    if (parent(node)) return get_alive_vars(parent(node), nodes, result);
    return;
  }
  }
}

void resolver::get_undefined_vars(ast::NodeList nodes, VariableList &result) {
  SymbolTable st;
  get_undefined_vars(nodes, st, result);
}
void resolver::get_undefined_vars(ast::Node node, VariableList &result) {
  SymbolTable st;
  get_undefined_vars(node, st, result);
}

static void process_ids(std::set<std::string> ids, Node node, SymbolTable &st, VariableList &result) {
  // FIXME cannot handle id htat is a type cast
  for (std::string id : ids) {
    if (is_c_keyword(id)) continue; // filter out c keyword
    if (st.LookUp(id)) continue;
    Variable v = resolver::resolve_var(node, id);
    // also add unique name, because I can't think up a scenario that an inner variable with the same name is not defined
    // and there's no way to init them anyhow.
    // The only problem that will arise is that the block is not continuous
    if (v) {
      add_unique(result, v);
      st.AddSymbol(v);        // should add this, treat as the variable is defined, to avoid future resolving the same variable.
    } else {
      // outputing some warning for unresolved ids
      // this can be some user-defined type, enums, in other (header) file
      // utils::print("Warning: cannot resolve: "+id, utils::CK_Red);
    }

  }
}

void resolver::get_undefined_vars(ast::Node node, SymbolTable &st, VariableList &result) {
  switch (kind(node)) {
  case NK_DeclStmt: {
    VariableList vars = var_from_node(node);
    st.AddSymbol(vars);
    // decl_stmt also contains some undefined variables
    std::set<std::string> ids = decl_stmt_get_var_ids(node);
    process_ids(ids, node, st, result);
    break;
  }
    /*******************************
     ** Control flow (will have next level)
     *******************************/
  case NK_For: {
    VariableList vars = var_from_node(node); // inside for init
    st.PushLevel();
    st.AddSymbol(vars);
    get_undefined_vars(for_get_init_decls(node), st, result);
    get_undefined_vars(for_get_condition_expr(node), st, result);
    get_undefined_vars(for_get_incr_expr(node), st, result);
    get_undefined_vars(for_get_block(node), st, result);
    st.PopLevel();
    break;
  }
  case NK_If: {
    st.PushLevel();
    get_undefined_vars(if_get_condition_expr(node), st, result);
    get_undefined_vars(if_get_then_block(node), st, result);
    get_undefined_vars(if_get_else_block(node), st, result);
    st.PopLevel();
    break;
  }
  case NK_While: {
    st.PushLevel();
    get_undefined_vars(while_get_condition_expr(node), st, result);
    get_undefined_vars(while_get_block(node), st, result);
    st.PopLevel();
    break;
  }
  case NK_Switch: {
    st.PushLevel();
    get_undefined_vars(switch_get_condition_expr(node), st, result);
    for (Node case_node : switch_get_cases(node)) {
      st.PushLevel();
      get_undefined_vars(case_get_nodes(case_node), st, result);
      st.PopLevel();
    }
    st.PopLevel();
    break;
  }
  case NK_Do: {
    st.PushLevel();
    get_undefined_vars(do_get_condition_expr(node), st, result);
    get_undefined_vars(do_get_block(node), st, result);
    st.PopLevel();
    break;
  }
  case NK_Expr: {
    // expr can contains multiple sub <expr>
    // e.g. <expr><call><arg><expr>
    std::set<std::string> ids = node_get_var_ids(node);
    process_ids(ids, node, st, result);
    break;
  }
  case NK_ExprStmt: {
    std::set<std::string> ids = expr_stmt_get_var_ids(node);
    process_ids(ids, node, st, result);
    break;
  }
  case NK_Block: {
    st.PushLevel();
    get_undefined_vars(block_get_nodes(node), st, result);
    st.PopLevel();
    break;
  }
  case NK_Decl: {
    std::set<std::string> ids = node_get_var_ids(node);
    process_ids(ids, node, st, result);
    for (std::string id : ids) {
      if (st.LookUp(id)) continue;
      Variable v = resolve_var(node, id);
      if (v) add_unique(result, v);
    }
    break;
  }
  default: {
    // TODO what to do here?
    // std::cerr <<"ignored node when getting undefined variable: "<< node.name() << '\n';
    // assert(false);
  }
  }
}
/**

 * Get a list of variables that is used in the node list, but not defined within them.
 
 * Top Down.

 * All variable usage is parsed as <name> tag in <expr> tag.
 * But, it is not marked as variable name, or function name, or other names.

 * We need to build the symbol table along the way.
 * Currently symbol table only contains variable.
 * Variables can be defined in
 *   - for init
 *   - decl_stmt
 *
 * from the first node in the node list, depth first search, maintain symbol table.
 * 
 * @param[in] nodes the nodes to search in this level
 * @param[in] the symbol table until this level
 * @param[in,out] the variable list that is undefined
 */
void resolver::get_undefined_vars(ast::NodeList nodes, SymbolTable &st, VariableList &result) {
  for (ast::Node node : nodes) {
    get_undefined_vars(node, st, result);
  }
}

