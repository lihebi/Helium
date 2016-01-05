#include "ast.h"

/*******************************
 ** DomUtil
 *******************************/



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

pugi::xml_node get_previous_ast_element(pugi::xml_node node) {
  while (node) {
    node = node.previous_sibling();
    if (node) {
      if (is_valid_ast(node.name())) {
        return node;
      }
    }
  }
  // This should be node_null
  return node;
}

pugi::xml_node get_parent_ast_element(pugi::xml_node node) {
  while (node) {
    node = node.parent();
    if (node) {
      if (is_valid_ast(node.name())) {
        return node;
      }
    }
  }
  return node;
}

/**
 * Get the call place of the function in node.
 * @param[in] node the <function> node in xml
 * @return the node <call> of the function. Or null_node is not found.
 */
// pugi::xml_node get_function_call(pugi::xml_node node) {
//   const char *func_name = node.child_value("name");
//   pugi::xpath_node_set call_nodes = node.root().select_nodes("//call");
//   for (auto it=call_nodes.begin();it!=call_nodes.end();it++) {
//     if (strcmp(it->node().child_value("name"), func_name) == 0) {
//       return get_parent_ast_element(it->node());
//     }
//   }
//   pugi::xml_node null_node;
//   return null_node;
// }

/**
 * Get text content of node.
 * Will traverse xml structure.
 * But will avoid a tag with "helium-omit" ATTR.
 * Add some structure to make the syntax correct. FIXME
 */
std::string
get_text_content(pugi::xml_node node) {
  std::string text;
  if (!node) return "";
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      if (!node.attribute("helium-omit")) {
        // add text only if it is not in helium-omit
        text += get_text_content(n);
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

/**
 * Get text content of node, except <name> tag
 */
std::string
get_text_content_except_tag(pugi::xml_node node, std::string name) {
  if (!node) return "";
  std::string text;
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      if (!node.attribute("helium-omit")) {
        if (strcmp(n.name(), name.c_str()) != 0) {
          text += get_text_content_except_tag(n, name);
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


// struct simple_walker: pugi::xml_tree_walker {
//   std::string text;
//   virtual bool for_each(pugi::xml_node& node) {
//     if (!node.attribute("helium-omit")) {
//       text += node.value();
//     }
//     return true; // continue traversal
//   }
// };

// std::string GetTextContent(pugi::xml_node node) {
//   simple_walker walker;
//   node.traverse(walker);
//   return walker.text;
// }



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
 * test if node is within <level> levels inside a <tagname>
 */
bool
dom_in_node(pugi::xml_node node, std::string tagname, int level) {
  while (node.parent() && level>0) {
    node = node.parent();
    level--;
    if (node.type() != pugi::node_element) return false;
    if (node.name() == tagname) return true;
  }
  return false;
}
