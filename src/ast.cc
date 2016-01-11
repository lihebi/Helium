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
    return kind_to_name_map.at(k);
  }

  
  NodeKind kind(Node node) {
    if (!node) return NK_Null;
    if (!node.name()) return NK_Null;
    const char *name = node.name();
    // if (name_to_kind.find(name) != name_to_kind.end()) return name_to_kind.at(name);
    try {
      return name_to_kind_map.at(name);
    } catch (const std::out_of_range& e) {
      assert(false && "should not reach here if I have a complete list.");
      return NK_Null;
    }
  }

  /**
   * Find the children of node of kind "kind".
   * It doesn't need to be direct child.
   */
  NodeList find_nodes(Node node, NodeKind kind) {
    NodeList result;
    std::string tag = "//";
    try {
      tag += kind_to_name_map.at(kind);
    } catch (const std::out_of_range& e) {
      return result;
    }
    pugi::xpath_node_set nodes = node.select_nodes(tag.c_str());
    for (auto it=nodes.begin();it!=nodes.end();++it) {
      result.push_back(it->node());
    }
    return result;
  }
  NodeList find_nodes(const Doc& doc, NodeKind kind) {
    Node root = doc.document_element();
    return find_nodes(root, kind);
  }
  NodeList find_nodes_from_root(Node node, NodeKind kind) {
    Node root = node.root();
    return find_nodes(root, kind);
  }

  /*******************************
   ** traversal
   *******************************/

  /**
   * Next sibling on AST level
   */
  Node next_sibling(Node node) {
    Node n = node;
    while ((n = n.next_sibling())) {
      if (is_valid_ast(n)) return n;
    }
    return Node();
  }
  Node previous_sibling(Node node) {
    Node n =node;
    while ((n = n.previous_sibling())) {
      if (is_valid_ast(n)) return n;
    }
    return Node();
  }
  Node parent(Node node) {
    Node n = node;
    while ((n = n.parent())) {
      if (is_valid_ast(n)) return n;
    }
    return Node();
  }

  /**
   * valid ast includes: expr, decl, break, macro, for, while, if, function
   */

  bool is_valid_ast(const char* name) {
    if (strcmp(name, "expr_stmt") == 0
        || strcmp(name, "decl_stmt") == 0
        || strcmp(name, "break") == 0
        || strcmp(name, "macro") == 0
        || strcmp(name, "for") == 0
        || strcmp(name, "while") == 0
        || strcmp(name, "if") == 0
        || strcmp(name, "function") == 0
        ) return true;
    else return false;
  }


  bool is_valid_ast(pugi::xml_node node) {
    return is_valid_ast(node.name());
  }

  /**
   * least upper bound of two nodes
   */
  pugi::xml_node
  lub(pugi::xml_node n1, pugi::xml_node n2) {
    if (n1.root() != n2.root()) return pugi::xml_node();
    pugi::xml_node root = n1.root();
    int num1=0, num2=0;
    pugi::xml_node n;
    n = n1;
    while (n!=root) {
      n = n.parent();
      num1++;
    }
    n = n2;
    while(n!=root) {
      n = n.parent();
      num2++;
    }
    if (num1 > num2) {
      // list 1 is longer
      while(num1-- != num2) {
        n1 = n1.parent();
      }
    } else {
      while(num2-- != num1) {
        n2 = n2.parent();
      }
    }
    // will end because the root is the same
    while (n1 != n2) {
      n1 = n1.parent();
      n2 = n2.parent();
    }
    return n1;
  }



  /**
   * Check if node is a sub node of any one of parent_nodes
   */
  bool contains(NodeList parent_nodes, Node node) {
    for (Node parent : parent_nodes) {
      if (contains(parent, node)) return true;
    }
    return false;
  }
  /**
   * Check if child is a sub node of parent
   */
  bool contains(Node parent, Node child) {
    if (lub(parent, child) == parent) return true;
    else return false;
  }

  int get_first_line_number(Node node) {
    Node n;
    try {
      n = node.select_node("//*[@pos::line]").node();
    } catch (pugi::xpath_exception) {
      // TODO
    }
    if (n) return atoi(n.attribute("pos::line").value());
    return -1;
  }
  
  std::string get_text(Node node) {
    std::string text;
    if (!node) return "";
    for (pugi::xml_node n : node.children()) {
      if (n.type() == pugi::node_element) {
        if (!node.attribute("helium-omit")) {
          // add text only if it is not in helium-omit
          text += get_text(n);
        } else if (strcmp(node.name(), "else") == 0 ||
                   strcmp(node.name(), "then") == 0 ||
                   strcmp(node.name(), "elseif") == 0 ||
                   strcmp(node.name(), "default") == 0
                   ) {
          // FIXME Why??????
          // For simplification of code.
          // I will add "helium-omit" attribute on the AST to mark deletion.
          // Those tag will be deleted.
          // But to make the syntax valid, I need to add some "{}"
          text += "{}";
        } else if (strcmp(node.name(), "case") == 0) {
          // FIXME why??????
          text += "{break;}";
        }
      } else {
        text += n.value();
      }
    }
    return text;
  }
  std::string get_text_except(Node node, std::string tag) {
    if (!node) return "";
    std::string text;
    for (pugi::xml_node n : node.children()) {
      if (n.type() == pugi::node_element) {
        if (!node.attribute("helium-omit")) {
          if (strcmp(n.name(), tag.c_str()) != 0) {
            text += get_text_except(n, tag);
          }
        }
        // TODO this version does not use the trick for simplification,
        // so it doesnot work with simplification
      } else {
        text += n.value();
      }
    }
    return text;
  }

  /**
   * True if node is inside a node of kind "kind"
   */
  bool in_node(Node node, NodeKind kind) {
    while ((node = parent(node))) {
      if (ast::kind(node) == kind) return true;
    }
    return false;
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
    for (Node param : parameter_list.children("param")) {
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



  TEST(ast_test_case, function_test) {
    Doc doc;
    const char *raw = R"prefix(

int myfunc(int a, int b) {
  int b;
}

)prefix";
    utils::string2xml(raw, doc);
    NodeList nodes = find_nodes(doc, NK_Function);
    EXPECT_EQ(nodes.size(), 1);
    Node myfunc = nodes[0];
    EXPECT_EQ(function_get_return_type(myfunc), "int");
    EXPECT_EQ(function_get_name(myfunc), "myfunc");
    NodeList params = function_get_params(myfunc);
    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(param_get_type(params[0]), "int");
    EXPECT_EQ(param_get_name(params[0]), "a");
    EXPECT_EQ(param_get_type(params[1]), "int");
    EXPECT_EQ(param_get_name(params[1]), "b");
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

  TEST(ast_test_case, decl_test) {
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

  // NodeList for_get_init_decls(Node node) {
  //   NodeList nodes;
  //   for (Node n : node.child("init").children("decl")) {
  //     nodes.push_back(n);
  //   }
  //   return nodes;
  // }
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

  TEST(ast_test_case, for_test) {
    Doc doc;
    const char *raw = R"prefix(

for (int i=0,c=2;i<8;++i) {
}

)prefix";
    utils::string2xml(raw, doc);
    NodeList nodes = find_nodes(doc, NK_For);
    ASSERT_EQ(nodes.size(), 1);
    Node myfor = nodes[0];
    std::map<std::string, std::string> vars = for_get_init_detail(myfor);
    ASSERT_EQ(vars.size(), 2);
    EXPECT_EQ(vars["i"], "int");
    EXPECT_EQ(vars["c"], "int");
    // Node condition_expr = for_get_condition_expr(myfor);
    // Node incr_expr = for_get_incr_expr(myfor);
    // Node block = for_get_block(myfor);
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
  

} // namespace ast ends here
