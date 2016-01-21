#include "ast.h"
#include "ast.h"
#include "gtest/gtest.h"
#include "utils.h"

using namespace ast;

static const std::map<NodeKind, std::string> kind_to_name_map {
  {NK_Function, "function"}
  , {NK_ArgList,  "argument_list"}
  , {NK_Arg,      "argument"}
  , {NK_DeclStmt, "decl_stmt"}
  , {NK_Decl,     "decl"}
  , {NK_ExprStmt, "expr_stmt"}
  , {NK_Expr,     "expr"}
  , {NK_For,      "for"}
  , {NK_Type,     "type"}
  , {NK_Name,     "name"}
  , {NK_Block,    "block"}
  , {NK_Stmt,     "stmt"}
  , {NK_Operator, "operator"}
  , {NK_Specifier,"specifier"}
  , {NK_Literal,  "literal"}
  , {NK_If,       "if"}
  , {NK_Condition, "condition"}
  , {NK_Then,     "then"}
  , {NK_Else,     "else"}
  , {NK_Case,     "case"}
  , {NK_Default,  "default"}
  , {NK_Switch,   "switch"}
  , {NK_While,    "while"}
  , {NK_Do,       "do"}
  , {NK_Call,     "call"}
  , {NK_ParamList, "parameter_list"}
  , {NK_Param,    "parameter"}
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
  , {NK_Include,  "cpp:include"}
  , {NK_Position, "pos:position"}
  , {NK_Unit,     "unit"}
  , {NK_Null,     "NULL"}

  , {NK_Extern,   "extern"}
  , {NK_EmptyStmt, "empty_stmt"}
  , {NK_FuncDecl, "function_decl"}
  , {NK_Modifier, "modifier"}
  , {NK_Range,    "range"}
  , {NK_StructDecl, "struct_decl"}
  , {NK_UnionDecl, "union_decl"}
  , {NK_Asm,      "asm"}
  , {NK_Op,       "op:operator"}
  , {NK_Sizeof,   "sizeof"}
};

static const std::map<std::string, NodeKind> name_to_kind_map {
  {"function",  NK_Function}
  , {"argument_list", NK_ArgList}
  , {"argument",  NK_Arg}
  , {"decl_stmt", NK_DeclStmt}
  , {"decl",      NK_Decl}
  , {"expr_stmt", NK_ExprStmt}
  , {"expr",      NK_Expr}
  , {"for",       NK_For}
  , {"type",      NK_Type}
  , {"name",      NK_Name}
  , {"block",     NK_Block}
  , {"stmt",      NK_Stmt}
  , {"operator",  NK_Operator}
  , {"specifier", NK_Specifier}
  , {"literal",   NK_Literal}
  , {"if",        NK_If}
  , {"condition", NK_Condition}
  , {"then",      NK_Then}
  , {"else",      NK_Else}
  , {"case",      NK_Case}
  , {"default",   NK_Default}
  , {"switch",    NK_Switch}
  , {"while",     NK_While}
  , {"do",        NK_Do}
  , {"call",      NK_Call}
  , {"parameter_list", NK_ParamList}
  , {"parameter",     NK_Param}
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
  , {"cpp:include", NK_Include}
  , {"pos:position", NK_Position}
  , {"unit"       , NK_Unit}
  , {"NULL",      NK_Null}
  
  , {"extern",    NK_Extern}
  , {"empty_stmt", NK_EmptyStmt}
  , {"function_decl", NK_FuncDecl}
  , {"modifier",  NK_Modifier}
  , {"range",    NK_Range}
  , {"struct_decl", NK_StructDecl}
  , {"union_decl", NK_UnionDecl}
  , {"asm",   NK_Asm}
  , {"op:operator", NK_Op}
  , {"sizeof",  NK_Sizeof}

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


std::string ast::kind_to_name(NodeKind k) {
  std::string result;
  try {
    result = kind_to_name_map.at(k);
  } catch (const std::out_of_range& e) {
    std::cerr << "Not supported kind.\n";
    assert(false);
  }
  return result;
}

  
NodeKind ast::kind(Node node) {
  if (!node) return NK_Null;
  if (!node.name()) return NK_Null;
  const char *name = node.name();
  if (strlen(name) == 0) return NK_Null;
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

/*******************************
 ** Expression <expr>
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

// std::set<std::string>
// ast::expr_stmt_get_ids(Node node) {
//   // TODO an expr_stmt can only have <expr> as child? Only have one?
//   Node expr = node.child("expr");
//   return expr_get_ids(expr);
// }

// /**
//  * Expr
//  */
// std::set<std::string> ast::expr_get_ids(Node node) {
//   std::set<std::string> result;
//   for (Node n : node.children("name")) {
//     std::string s = get_text(n);
//     simplify_variable_name(s);
//     result.insert(s);
//   }
//   return result;
// }

std::set<std::string> ast::expr_stmt_get_var_ids(Node node) {
  pugi::xpath_node_set expr_nodes = node.select_nodes(".//expr");
  std::set<std::string> result;
  for (auto n : expr_nodes) {
    std::set<std::string> tmp = expr_get_var_ids(n.node());
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}

std::set<std::string> ast::expr_get_var_ids(Node node) {
  std::set<std::string> result;
  for (Node n : node.children("name")) {
    std::string s = get_text(n);
    simplify_variable_name(s);
    result.insert(s);
  }
  return result;
}

std::set<std::string> ast::get_var_ids(Node node) {
  std::set<std::string> result;
  for (auto name_node : node.select_nodes(".//expr/name")) {
    result.insert(name_node.node().child_value());
  }
  return result;
}

std::set<std::string> ast::get_var_ids(NodeList nodes) {
  std::set<std::string> result;
  for (Node n : nodes) {
    std::set<std::string> tmp = get_var_ids(n);
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}

/**
 * //type/name
 */
std::set<std::string> ast::get_type_ids(Node node) {
  std::set<std::string> result;
  for (auto name_node : node.select_nodes(".//type/name")) {
    result.insert(name_node.node().child_value());
  }
  return result;
}
std::set<std::string> ast::get_type_ids(NodeList nodes) {
  std::set<std::string> result;
  for (Node n : nodes) {
    std::set<std::string> tmp = get_type_ids(n);
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}


/**
 * //call/name
 */
std::set<std::string> ast::get_call_ids(Node node) {
  std::set<std::string> result;
  for (auto name_node : node.select_nodes(".//call/name")) {
    result.insert(name_node.node().child_value());
  }
  return result;
}
std::set<std::string> ast::get_call_ids(NodeList nodes) {
  std::set<std::string> result;
  for (Node n : nodes) {
    std::set<std::string> tmp = get_call_ids(n);
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}


/**
 * decl_stmt's variables except the declared variables.
 * e.g. int a = b; int a = func(b,c);
 */
std::set<std::string> ast::decl_stmt_get_var_ids(Node node) {
  std::set<std::string> result;
  pugi::xpath_node_set expr_nodes = node.select_nodes(".//expr");
  for (auto n : expr_nodes) {
    std::set<std::string> tmp = expr_get_var_ids(n.node());
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}

/**
 * for general node, search for //expr, and resolve /name
 */
std::set<std::string> ast::node_get_var_ids(Node node) {
  std::set<std::string> result;
  pugi::xpath_node_set expr_nodes = node.select_nodes(".//expr");
  for (auto n : expr_nodes) {
    std::set<std::string> tmp = expr_get_var_ids(n.node());
    result.insert(tmp.begin(), tmp.end());
  }
  return result;
}


/*******************************
 ** Function
 *******************************/

/**
 * Call
 */
std::string ast::call_get_name(Node node) {
  return node.child_value("name");
}


/**
 * decl_stmt
 */
NodeList ast::decl_stmt_get_decls(Node node) {
  NodeList nodes;
  for (Node n : node.children("decl")) {
    nodes.push_back(n);
  }
  return nodes;
}

std::string ast::decl_get_name(Node node) {
  // this is an array
  if (node.child("name").child("name")) {
    return node.child("name").child_value("name");
  }
  // has a <decl><name>xxx</name></decl>
  return node.child_value("name");
}

std::string ast::decl_get_type(Node node) {
  // if (!node || strcmp(node.name(), "decl") != 0) {
  //   assert(false && "node should be <decl>");
  // }
  if (node.child("type").attribute("ref")) {
    assert(strcmp(node.child("type").attribute("ref").value(), "prev") == 0);
    // go to parent <decl_stmt>
    // get the first <decl>'s type
    // assert it should not be empty
    Node decl_stmt_node = node.parent();
    std::string type = decl_stmt_node.child("decl").child("type").child_value("name");
    // then, count how many <modifier>s direct before, until a <decl>
    // CAUTION: <modifier> may contain &
    for (Node n=node.previous_sibling();kind(n)!=NK_Decl;n=n.previous_sibling()) {
      if (kind(n) == NK_Modifier && strcmp(n.child_value(), "*")==0) {
        type +='*';
      }
    }
    return type;
  }
  // return node.child("type").child_value("name");
  // return get_text(node.child("type")); // this version contains modifiers
  // this version is even better:
  // 1) will append '*' directly to the type name, without space
  // 2) better control of the content of node, incase there's other unexpected item inside <type>
  std::string type = node.child("type").child_value("name");
  for (Node n : node.child("type").children("modifier")) {
    if (strcmp(n.child_value(), "*") == 0) {
      type += '*';
    }
  }
  // array
  if (node.child("name").child("index")) {
    for (Node n : node.child("name").children("index")) {
      n.value(); // to suppress unused variable warning
      type += "[]";
    }
  }
  return type;
}


  
/**
 * block
 */
NodeList ast::block_get_nodes(Node node) {
  NodeList result;
  for (Node n : node.children()) {
    if (n.type() == pugi::node_element) result.push_back(n);
  }
  return result;
}



/**
 * Function
 */
  
std::string ast::function_get_return_type(Node node) {
  return node.child("type").child_value("name");
}
  
std::string ast::function_get_name(Node node) {
  return node.child_value("name");
}
  
NodeList ast::function_get_params(Node node) {
  NodeList nodes;
  Node parameter_list = node.child("parameter_list");
  for (Node param : parameter_list.children("parameter")) {
    nodes.push_back(param);
  }
  return nodes;
}
Node ast::function_get_block(Node node) {
  return node.child("block");
}

std::string ast::param_get_type(Node node) {
  // FIXME should contains modifiers, pointers, etc. Or should do semantic analysis further?
  return node.child("decl").child("type").child_value("name");
}
std::string ast::param_get_name(Node node) {
  return node.child("decl").child_value("name");
}




/**
 * get a map from name to type.
 * e.g. int a,b; char c;
 * will return:
 * {(a,int), (b,int), (c,char)}
 */
// std::map<std::string, std::string> get_decl_detail(std::string code) {
//   std::map<std::string, std::string> result;
//   utils::trim(code);
//   if (code.back() != ';') code += ';';
//   Doc doc;
//   utils::string2xml(code, doc);
//   NodeList decl_stmts = find_nodes(doc, NK_DeclStmt);
//   for (Node decl_stmt : decl_stmts) {
//     std::string type = decl_stmt.child("decl").child("type").child_value("name");
//     for (Node decl : decl_stmt.children("decl")) {
//       for (Node name : decl.children("name")) {
//         std::string s = name.child_value();
//         result[s] = type;
//       }
//     }
//   }
//   return result;
// }

/**
 * This node can be <decl_stmt>, or <init>(for), both of which may contains a list of <decl>s
 * deprecated.
 */
// std::map<std::string, std::string> get_decl_detail(Node node) {
//   std::map<std::string, std::string> result;
//   std::string type = node.child("decl").child("type").child_value("name");
//   if (type.empty()) return result;
//   for (Node decl : node.children("decl")) {
//     for (Node name : decl.children("name")) {
//       std::string s = name.child_value();
//       result[s] = type;
//     }
//   }
//   return result;
// }


  
/**
 * For
 */

NodeList ast::for_get_init_decls(Node node) {
  NodeList nodes;
  for (Node n : node.child("control").child("init").children("decl")) {
    nodes.push_back(n);
  }
  return nodes;
}
  
// std::map<std::string, std::string> ast::for_get_init_detail(Node node) {
//   return get_decl_detail(node.child("init"));
// }
Node ast::for_get_condition_expr(Node node) {
  return node.child("condition").child("expr");
}
Node ast::for_get_incr_expr(Node node) {
  return node.child("incr").child("expr");
}
Node ast::for_get_block(Node node) {
  return node.child("block");
}



/*******************************
 ** If
 *******************************/
Node ast::if_get_condition_expr(Node node) {
  return node.child("condition").child("expr");
}
Node ast::if_get_then_block(Node node) {
  return node.child("then").child("block");
}

Node ast::if_get_else_block(Node node) {
  return node.child("else").child("block");
}

/*******************************
 ** Switch
 *******************************/

Node ast::switch_get_condition_expr(Node node) {
  return node.child("condition").child("expr");
}
NodeList ast::switch_get_cases(Node node) {
  NodeList result;
  for (Node n : node.child("block").children("case")) {
    result.push_back(n);
  }
  return result;
}
NodeList ast::case_get_nodes(Node node) {
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
Node ast::do_get_condition_expr(Node node) {
  return node.child("condition").child("expr");
}
Node ast::do_get_block(Node node) {
  return node.child("block");
}

/*******************************
 ** While
 *******************************/
ast::Node ast::while_get_condition_expr(ast::Node node) {
  return node.child("condition").child("expr");
}
ast::Node ast::while_get_block(ast::Node node) {
  return node.child("block");
}


  

