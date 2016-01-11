#ifndef AST_H
#define AST_H

#include <pugixml.hpp>
#include "common.h"

namespace ast {

  typedef pugi::xml_node Node;
  typedef pugi::xml_document Doc;
  typedef std::vector<Node> NodeList;

  typedef enum _NodeKind {
    NK_Function,
    NK_DeclStmt,
    NK_Decl,
    NK_ExprStmt,
    NK_Expr,
    NK_For,
    NK_Type,
    NK_Block,
    NK_Stmt,
    NK_If,
    NK_Case,
    NK_Default,
    NK_Switch,
    NK_While,
    NK_Do,
    NK_Call,
    NK_Param,
    NK_Break,
    NK_Continue,
    NK_Return,
    NK_Label,
    NK_Goto,
    NK_Typedef,
    NK_Struct,
    NK_Union,
    NK_Enum,
    NK_Comment,
    NK_Null
  } NodeKind;
  std::string kind_to_name(NodeKind k);  

  /*******************************
   ** Help function
   *******************************/
  bool is_valid_ast(pugi::xml_node node);

  NodeList find_nodes(Node node, NodeKind kind);
  NodeList find_nodes(const Doc& doc, NodeKind kind);
  NodeList find_nodes_from_root(Node node, NodeKind kind);

  Node next_sibling(Node node);
  Node previous_sibling(Node node);
  Node parent(Node node);

  /**
   * Check if node is a sub node of any one of parent_nodes
   */
  bool contains(NodeList parent_nodes, Node node);
  /**
   * Check if child is a sub node of parent
   */
  bool contains(Node parent, Node child);

  int get_first_line_number(Node n);
  std::string get_text(Node n);
  std::string get_text_except(Node n, std::string tag);

  /**
   * True if node is inside a node of kind "kind"
   */
  bool in_node(Node node, NodeKind kind);

  NodeKind kind(Node node);
  /*******************************
   ** For specific type of node
   *******************************/

  // TODO validate node
  
  std::string function_get_return_type(Node node);
  std::string function_get_name(Node node);
  NodeList function_get_params(Node node);
  Node function_get_block(Node node);

  std::string param_get_type(Node node);
  std::string param_get_name(Node node);
  NodeList block_get_nodes(Node node);
  
  NodeList decl_stmt_get_decls(Node node);
  // FIXME srcml will give one <decl> for multiple variable, int a=0,b=8;
  std::string decl_get_name(Node node);
  std::string decl_get_type(Node node);

  // NodeList for_get_init_decls(Node node);
  std::map<std::string, std::string> for_get_init_detail(Node node);
  Node for_get_condition_expr(Node node);
  Node for_get_incr_expr(Node node);
  Node for_get_block(Node node);
  
  // if
  Node if_get_then_block(Node node);
  Node if_get_else_block(Node node);

  // switch
  Node switch_get_condition_expr(Node);
  NodeList switch_get_cases(Node);
  NodeList case_get_nodes(Node);
  
  // do
  Node do_get_condition_expr(Node);
  Node do_get_block(Node);

  // while
  Node while_get_condition_expr(Node);
  Node while_get_block(Node);
  
  std::set<std::string> expr_get_ids(Node);

  std::string call_get_name(Node);

}// end namespace ast

#endif /* AST_H */
