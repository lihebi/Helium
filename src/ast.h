#ifndef AST_H
#define AST_H

#include <pugixml.hpp>
#include "common.h"

namespace ast {

  typedef pugi::xml_node Node;
  typedef pugi::xml_node XMLNode;
  typedef pugi::xml_document Doc;
  typedef pugi::xml_document XMLDoc;
  typedef std::vector<Node> NodeList;
  typedef NodeList XMLNodeList;

  typedef enum _NodeKind {
    // general structure
    NK_DeclStmt,
    NK_Decl,
    NK_Init,
    NK_ExprStmt,
    NK_Expr,
    NK_EmptyStmt,
    // more general
    NK_Type,
    NK_Name,
    NK_Block,
    NK_Stmt,
    NK_Operator,
    NK_Op, // why op:operator?
    NK_Asm,
    NK_Literal,
    NK_Specifier,
    NK_Modifier,
    NK_Extern,
    NK_Range, // bit field
    NK_Sizeof,
    // control flow
    NK_For,
    NK_Control,
    NK_ForIncr,
    NK_If,
    NK_Condition,
    NK_Then,
    NK_Else,
    NK_ElseIf,
    NK_Case,
    NK_Default,
    NK_Switch,
    NK_While,
    NK_Do,
    NK_Break,
    NK_Continue,
    NK_Return,
    NK_Label,
    NK_Goto,
    // function
    NK_Call,
    NK_Function,
    NK_FuncDecl,
    NK_ArgList,
    NK_Arg,
    NK_ParamList,
    NK_Param,
    // structures
    NK_Typedef,
    NK_Struct,
    NK_StructDecl,
    NK_Union,
    NK_UnionDecl,
    NK_Enum,
    NK_Define,
    // conditional compile
    NK_CppIf,
    NK_CppElif,
    NK_IfDef,
    NK_IfnDef,
    NK_DefElse,
    NK_EndIf,
    NK_Include,
    NK_Pragma,
    NK_Directive,
    NK_Undef,
    // trival srcml staff
    NK_Comment,
    NK_Position,
    NK_Unit,
    NK_HeliumInstrument,
    NK_Null,
    NK_Other, // other name
    NK_Invalid // the node is invalid
  } NodeKind;
  std::string kind_to_name(NodeKind k);  
  NodeKind kind(Node node);

  const std::set<NodeKind> helium_valid_ast {
    NK_Function,
      NK_DeclStmt,
      NK_ExprStmt,
      NK_For,
      // NK_Block,
      NK_Stmt,
      NK_If,
      NK_Switch,
      NK_While,
      NK_Do,
      NK_Break,
      NK_Continue,
      NK_Return,
      NK_Typedef,
      NK_Struct,
      NK_Union,
      NK_Enum,
      // NK_Comment,
      NK_Define
  };


  /*******************************
   ** For specific type of node
   *******************************/

  // get the filename field of this SrcML document belongs
  std::string unit_get_filename(XMLNode node);

  // TODO validate node
  
  std::string function_get_return_type(Node node);
  std::string function_get_name(Node node);
  NodeList function_get_params(Node node);
  Node function_get_block(Node node);

  Node get_function_node(Node node);

  NodeList function_get_param_decls(Node node);

  std::string get_function_decl(std::string code);


  // deprecated
  std::string param_get_type(Node node);
  std::string param_get_name(Node node);
  // use this one
  Node param_get_decl(Node node);
  NodeList block_get_nodes(Node node);


  NodeList decl_stmt_get_decls(Node node);
  // FIXME srcml will give one <decl> for multiple variable, int a=0,b=8;
  std::string decl_get_name(Node node);
  std::string decl_get_type(Node node);

  std::vector<std::string> decl_get_dimension(Node node);


  NodeList for_get_init_decls_or_exprs(Node node);
  // std::map<std::string, std::string> for_get_init_detail(Node node);
  Node for_get_condition_expr(Node node);
  Node for_get_incr_expr(Node node);
  Node for_get_block(Node node);
  Node for_get_control(Node node);  
  // if
  Node if_get_condition_expr(Node node);
  Node if_get_then_block(Node node);
  Node if_get_else_block(Node node);
  Node if_get_then(Node node);
  Node if_get_else(Node node);
  NodeList if_get_elseifs(Node node);

  Node then_get_block(Node node);
  Node else_get_block(Node node);
  Node elseif_get_block(Node node);
  // elseif IF related
  Node elseif_get_condition_expr(Node node);
  

  // switch
  Node switch_get_condition_expr(Node);
  NodeList switch_get_cases(Node);
  Node switch_get_default(Node node);
  Node case_get_condition_expr(Node node);

  
  NodeList case_get_nodes(Node);
  NodeList switch_get_blocks(Node node);
  Node switch_get_block(Node node);
  // NodeList switch_get_content(Node node);
  // do
  Node do_get_condition_expr(Node);
  Node do_get_block(Node);

  // while
  Node while_get_condition_expr(Node);
  Node while_get_block(Node);

  // expr
  // std::set<std::string> expr_stmt_get_ids(Node);
  // std::set<std::string> expr_get_ids(Node);
  std::set<std::string> expr_stmt_get_var_ids(Node);
  std::set<std::string> expr_get_var_ids(Node);
  std::set<std::string> decl_stmt_get_var_ids(Node node);  
  std::set<std::string> node_get_var_ids(Node node);
  // the above are all deprecated
  std::set<std::string> get_var_ids(Node node);
  std::set<std::string> get_var_ids(NodeList nodes);

  std::set<std::string> get_type_ids(Node node);
  std::set<std::string> get_type_ids(NodeList nodes);

  std::set<std::string> get_call_ids(Node node);
  std::set<std::string> get_call_ids(NodeList nodes);
  
  std::string call_get_name(Node);

  /*******************************
   ** Help function
   *******************************/
  bool is_valid_ast(Node node);


  Node next_sibling(Node node);
  Node previous_sibling(Node node);
  Node parent(Node node);

  // helium specific ast
  // using helium_valid_ast
  bool helium_is_valid_ast(Node n);
  Node helium_next_sibling(Node n);
  Node helium_previous_sibling(Node n);
  Node helium_parent(Node n);

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
  std::string get_text_ln(NodeList nodes);


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

  // based on content (mainly comment)
  Node find_node_containing_str(Node node, NodeKind k, std::string s);
  NodeList find_nodes_containing_str(Node node, NodeKind k, std::string s);
  Node find_node_containing_str(const Doc &doc, NodeKind k, std::string s);
  NodeList find_nodes_containing_str(const Doc &doc, NodeKind k, std::string s);

  /**
   * Find the first <call> whose name is func.
   */
  Node find_callsite(pugi::xml_document &doc, std::string func);
  Node find_callsite(pugi::xml_node node, std::string func);
  NodeList find_callsites(pugi::xml_node node, std::string func);
  

  std::string get_code_enclosing_line(const std::string& filename, int line_number, std::string tag_name);

  // srcml specific
  std::string get_filename(Node node);
  std::string get_filename(Doc &doc);

}// end namespace ast

#endif /* AST_H */
