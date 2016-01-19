#include "ast.h"
#include "ast.h"
#include "gtest/gtest.h"
#include "utils.h"

namespace ast {

  static const std::map<NodeKind, std::string> kind_to_name_map {
    {NK_Function, "function"}
    , {NK_DeclStmt, "decl_stmt"}
    , {NK_Decl,     "decl"}
    , {NK_ExprStmt, "expr_stmt"}
    , {NK_Expr,     "expr"}
    , {NK_For,      "for"}
    , {NK_Type,     "type"}
    , {NK_Block,    "block"}
    , {NK_Stmt,     "stmt"}
    , {NK_If,       "if"}
    , {NK_Case,     "case"}
    , {NK_Default,  "default"}
    , {NK_Switch,   "switch"}
    , {NK_While,    "while"}
    , {NK_Do,       "do"}
    , {NK_Call,     "call"}
    , {NK_Param,    "param"}
    , {NK_Break,    "break"}
    , {NK_Continue, "continue"}
    , {NK_Return,   "return"}
    , {NK_Label,    "label"}
    , {NK_Goto,     "goto"}
    , {NK_Typedef,  "typedef"}
    , {NK_Struct,   "struct"}
    , {NK_Union,    "union"}
    , {NK_Enum,     "enum"}
    , {NK_Comment,  "comment"}
    , {NK_Define,   "cpp:define"}
    , {NK_IfnDef,   "cpp:ifndef"}
    , {NK_IfDef,    "cpp:ifdef"}
    , {NK_DefElse,  "cpp:else"}
    , {NK_EndIf,    "cpp:endif"}
    , {NK_Null,     "NULL"}
  };

  static const std::map<std::string, NodeKind> name_to_kind_map {
    {"function",  NK_Function}
    , {"decl_stmt", NK_DeclStmt}
    , {"decl",      NK_Decl}
    , {"expr_stmt", NK_ExprStmt}
    , {"expr",      NK_Expr}
    , {"for",       NK_For}
    , {"type",      NK_Type}
    , {"block",     NK_Block}
    , {"stmt",      NK_Stmt}
    , {"if",        NK_If}
    , {"case",      NK_Case}
    , {"default",   NK_Default}
    , {"switch",    NK_Switch}
    , {"while",     NK_While}
    , {"do",        NK_Do}
    , {"call",      NK_Call}
    , {"param",     NK_Param}
    , {"break",     NK_Break}
    , {"continue",  NK_Continue}
    , {"return",    NK_Return}
    , {"label",     NK_Label}
    , {"goto",      NK_Goto}
    , {"typedef",   NK_Typedef}
    , {"struct",    NK_Struct}
    , {"union",     NK_Union}
    , {"enum",      NK_Enum}
    , {"comment",   NK_Comment}
    , {"cpp:define",    NK_Define}
    , {"cpp:ifndef", NK_IfnDef}
    , {"cpp:ifdef", NK_IfDef}
    , {"cpp:else",  NK_DefElse}
    , {"cpp:endif", NK_EndIf}
    , {"NULL",      NK_Null}
  };
  /*
   * To make sure the above two mapping are consistant
   */
  TEST(ast_test_case, kind_name_test) {
    ASSERT_EQ(kind_to_name_map.size(), name_to_kind_map.size());
    for (auto m : kind_to_name_map) {
      EXPECT_EQ(name_to_kind_map.at(m.second), m.first);
    }
  }


  std::string kind_to_name(NodeKind k) {
    std::string result;
    try {
      result = kind_to_name_map.at(k);
    } catch (const std::out_of_range& e) {
      std::cerr << "Not supported kind.\n";
      assert(false);
    }
    return result;
  }

  
  NodeKind kind(Node node) {
    if (!node) return NK_Null;
    if (!node.name()) return NK_Null;
    const char *name = node.name();
    // if (name_to_kind.find(name) != name_to_kind.end()) return name_to_kind.at(name);
    try {
      return name_to_kind_map.at(name);
    } catch (const std::out_of_range& e) {
      std::cerr << name << " is not handled." << "\n";
      std::cout << "-- text:" << "\n";
      std::cout << get_text(node) << "\n";
      std::cout << "-- node name:" << "\n";
      std::cout << node.name() << "\n";
      std::cout << "-- xml structure:" << "\n";
      node.print(std::cout);
      // std::cout << "-- parent:" << "\n";
      // node.parent().print(std::cout);
      assert(false && "should not reach here if I have a complete list.");
      return NK_Null;
    }
  }

  /*******************************
   ** Specific node
   *******************************/

  
  static void
  simplify_variable_name(std::string& s) {
    s = s.substr(0, s.find('['));
    s = s.substr(0, s.find("->"));
    s = s.substr(0, s.find('.'));
    // TODO wiki the erase-remove-idiom
    s.erase(std::remove(s.begin(), s.end(), '('), s.end());
    s.erase(std::remove(s.begin(), s.end(), ')'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '*'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '&'), s.end());
  }

  /**
   * Expr
   */
  std::set<std::string> expr_get_ids(Node node) {
    std::set<std::string> result;
    for (Node n : node.children("name")) {
      std::string s = get_text(n);
      simplify_variable_name(s);
      result.insert(s);
    }
    return result;
  }

  /**
   * Call
   */
  std::string call_get_name(Node node) {
    return node.child_value("name");
  }


  /**
   * decl_stmt
   */
  NodeList decl_stmt_get_decls(Node node) {
    NodeList nodes;
    for (Node n : node.children("decl")) {
      nodes.push_back(n);
    }
    return nodes;
  }

  std::string decl_get_name(Node node) {
    return node.child_value("name");
  }
  std::string decl_get_type(Node node) {
    // if (!node || strcmp(node.name(), "decl") != 0) {
    //   assert(false && "node should be <decl>");
    // }
    if (node.child("type").attribute("ref")) {
      assert(strcmp(node.child("type").attribute("ref").value(), "prev") == 0);
      Node previous_decl = previous_sibling(node);
      return decl_get_type(previous_decl);
    }
    return node.child("type").child_value("name");
  }


  
  /**
   * block
   */
  NodeList block_get_nodes(Node node) {
    NodeList result;
    for (Node n : node.children()) {
      if (n.type() == pugi::node_element) result.push_back(n);
    }
    return result;
  }



  /**
   * Function
   */
  
  std::string function_get_return_type(Node node) {
    return node.child("type").child_value("name");
  }
  
  std::string function_get_name(Node node) {
    return node.child_value("name");
  }
  
  NodeList function_get_params(Node node) {
    NodeList nodes;
    Node parameter_list = node.child("parameter_list");
    for (Node param : parameter_list.children("parameter")) {
      nodes.push_back(param);
    }
    return nodes;
  }
  Node function_get_block(Node node) {
    return node.child("block");
  }

  std::string param_get_type(Node node) {
    // FIXME should contains modifiers, pointers, etc. Or should do semantic analysis further?
    return node.child("decl").child("type").child_value("name");
  }
  std::string param_get_name(Node node) {
    return node.child("decl").child_value("name");
  }




  /**
   * get a map from name to type.
   * e.g. int a,b; char c;
   * will return:
   * {(a,int), (b,int), (c,char)}
   */
  std::map<std::string, std::string> get_decl_detail(std::string code) {
    std::map<std::string, std::string> result;
    utils::trim(code);
    if (code.back() != ';') code += ';';
    Doc doc;
    utils::string2xml(code, doc);
    NodeList decl_stmts = find_nodes(doc, NK_DeclStmt);
    for (Node decl_stmt : decl_stmts) {
      std::string type = decl_stmt.child("decl").child("type").child_value("name");
      for (Node decl : decl_stmt.children("decl")) {
        for (Node name : decl.children("name")) {
          std::string s = name.child_value();
          result[s] = type;
        }
      }
    }
    return result;
  }

  /**
   * This node can be <decl_stmt>, or <init>(for), both of which may contains a list of <decl>s
   * deprecated.
   */
  std::map<std::string, std::string> get_decl_detail(Node node) {
    std::map<std::string, std::string> result;
    std::string type = node.child("decl").child("type").child_value("name");
    if (type.empty()) return result;
    for (Node decl : node.children("decl")) {
      for (Node name : decl.children("name")) {
        std::string s = name.child_value();
        result[s] = type;
      }
    }
    return result;
  }

  TEST(ast_test_case, decl_test_deprecated) {
    std::string code;
    std::map<std::string, std::string> decls;
    // single
    code = "int a;";
    decls = get_decl_detail(code);
    EXPECT_EQ(decls.size(), 1);
    EXPECT_EQ(decls["a"], "int");
    // without ;
    code = "int a";
    decls = get_decl_detail(code);
    EXPECT_EQ(decls.size(), 1);
    EXPECT_EQ(decls["a"], "int");
    // double ;;
    code = "int a;;";
    decls = get_decl_detail(code);
    EXPECT_EQ(decls.size(), 1);
    EXPECT_EQ(decls["a"], "int");
    // double
    code = "int a;char b;";
    decls = get_decl_detail(code);
    EXPECT_EQ(decls.size(), 2);
    EXPECT_EQ(decls["a"], "int");
    EXPECT_EQ(decls["b"], "char");
    // same type
    code = "int a,b;";
    decls = get_decl_detail(code);
    EXPECT_EQ(decls.size(), 2);
    EXPECT_EQ(decls["a"], "int");
    EXPECT_EQ(decls["b"], "int");
  }

  
  /**
   * For
   */

  NodeList for_get_init_decls(Node node) {
    NodeList nodes;
    for (Node n : node.child("control").child("init").children("decl")) {
      nodes.push_back(n);
    }
    return nodes;
  }
  
  std::map<std::string, std::string> for_get_init_detail(Node node) {
    return get_decl_detail(node.child("init"));
  }
  Node for_get_condition_expr(Node node) {
    return node.child("condition").child("expr");
  }
  Node for_get_incr_expr(Node node) {
    return node.child("incr").child("expr");
  }
  Node for_get_block(Node node) {
    return node.child("block");
  }



  /*******************************
   ** If
   *******************************/
  Node if_get_then_block(Node node) {
    return node.child("then").child("block");
  }

  Node if_get_else_block(Node node) {
    return node.child("else").child("block");
  }

  /*******************************
   ** Switch
   *******************************/

  Node switch_get_condition_expr(Node node) {
    return node.child("condition").child("expr");
  }
  NodeList switch_get_cases(Node node) {
    NodeList result;
    for (Node n : node.child("block").children("case")) {
      result.push_back(n);
    }
    return result;
  }
  NodeList case_get_nodes(Node node) {
    NodeList result;
    if (node.child("block")) {
      // FIXME this child may be a inner block, not the first one
      Node block = node.child("block");
      result = block_get_nodes(block);
    } else {
      // the first expr and a : are conditino.
      // TODO
    }
    return result;
  }

  /*******************************
   ** Do
   *******************************/
  Node do_get_condition_expr(Node node) {
    return node.child("condition").child("expr");
  }
  Node do_get_block(Node node) {
    return node.child("block");
  }

  /*******************************
   ** While
   *******************************/
  Node while_get_condition_expr(Node node) {
    return node.child("condition").child("expr");
  }
  Node while_get_block(Node node) {
    return node.child("block");
  }


  

} // namespace ast ends here
