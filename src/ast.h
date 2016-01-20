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
    NK_Define,
    NK_IfDef,
    NK_IfnDef,
    NK_DefElse,
    NK_EndIf,
    NK_Null
  } NodeKind;
  std::string kind_to_name(NodeKind k);  
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

  NodeList for_get_init_decls(Node node);
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

  /*******************************
   ** Help function
   *******************************/
  bool is_valid_ast(pugi::xml_node node);


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

  std::string get_text(Node n);
  std::string get_text(NodeList nodes);

  // deprecated
  // std::string get_text_except(Node n, std::string tag);
  // std::string get_text_except(NodeList nodes, std::string tag);

  std::string get_text_except(Node n, NodeKind k);
  std::string get_text_except(Node n, std::vector<NodeKind> kinds);
  std::string get_text_except(NodeList nodes, NodeKind k);
  std::string get_text_except(NodeList nodes, std::vector<NodeKind> kinds);

  /**
   * True if node is inside a node of kind "kind"
   */
  bool in_node(Node node, NodeKind kind);


  int get_node_line(pugi::xml_node node);
  int get_node_last_line(pugi::xml_node node);

  /*******************************
   ** find nodes
   *******************************/
  // by kind
  NodeList find_nodes(Node node, NodeKind kind);
  NodeList find_nodes(const Doc& doc, NodeKind kind);
  NodeList find_nodes_from_root(Node node, NodeKind kind);
  // by kinds
  NodeList find_nodes(Node node, std::vector<NodeKind> kinds);
  NodeList find_nodes(const Doc& doc, std::vector<NodeKind> kinds);
  NodeList find_nodes_from_root(Node node, std::vector<NodeKind> kinds);

  // based on line
  Node find_node_on_line(Node node, NodeKind k, int line_number);
  Node find_node_on_line(Node node, std::vector<NodeKind> kinds, int line_number);
  NodeList find_nodes_on_lines(Node node, NodeKind k, std::vector<int> lines);
  NodeList find_nodes_on_lines(Node node, std::vector<NodeKind> kinds, std::vector<int> lines);
  // enclosing line
  // TODO kinds
  Node find_node_enclosing_line(Node node, NodeKind k, int line_number);
  Node find_outer_node_enclosing_line(Node node, NodeKind k, int line_number);

  std::string get_code_enclosing_line(const std::string& filename, int line_number, std::string tag_name);

}// end namespace ast

#endif /* AST_H */
