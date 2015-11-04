#include <util/DomUtil.hpp>
#include <iostream>
#include <cstring>
#include <cassert>

void DomUtil::GetFirstChildByTagName(std::string) {}
void DomUtil::GetFirstChildByTagNames(std::string) {}
void DomUtil::GetChildrenByTagName(std::string) {}

bool isValidAST(const char* name) {
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
pugi::xml_node DomUtil::GetPreviousASTElement(pugi::xml_node node) {
  while (node) {
    node = node.previous_sibling();
    if (node) {
      if (isValidAST(node.name())) {
        return node;
      }
    }
  }
  // This should be node_null
  return node;
}
pugi::xml_node DomUtil::GetParentASTElement(pugi::xml_node node) {
  while (node) {
    node = node.parent();
    if (node) {
      if (isValidAST(node.name())) {
        return node;
      }
    }
  }
  return node;
}
pugi::xml_node DomUtil::GetFunctionCall(pugi::xml_node node) {
  const char *func_name = node.child_value("name");
  pugi::xpath_node_set call_nodes = node.root().select_nodes("//call");
  for (auto it=call_nodes.begin();it!=call_nodes.end();it++) {
    if (strcmp(it->node().child_value("name"), func_name) == 0) {
      std::cout<<"[DomUtil] found function call to "<<func_name<<std::endl;
      return GetParentASTElement(it->node());
    }
  }
  pugi::xml_node null_node;
  return null_node;
}

std::string
DomUtil::GetTextContent(pugi::xml_node node) {
  std::string text;
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      if (!node.attribute("helium-omit")) {
        // add text only if it is not in helium-omit
        text += GetTextContent(n);
      } else if (strcmp(node.name(), "else") == 0 ||
        strcmp(node.name(), "then") == 0 ||
        strcmp(node.name(), "elseif") == 0 ||
        strcmp(node.name(), "default") == 0
      ) {
        text += "{}";
      } else if (strcmp(node.name(), "case") == 0) {
        text += "{break;}";
      }
    } else {
      text += n.value();
    }
  }
  return text;
}


struct simple_walker: pugi::xml_tree_walker {
  std::string text;
  virtual bool for_each(pugi::xml_node& node) {
    if (!node.attribute("helium-omit")) {
      text += node.value();
    }
    return true; // continue traversal
  }
};

// std::string DomUtil::GetTextContent(pugi::xml_node node) {
//   simple_walker walker;
//   node.traverse(walker);
//   return walker.text;
// }


/*
 * Test if node is in a <tagname> within <level> levels
 */
bool
DomUtil::InNode(pugi::xml_node node, std::string tagname, int level) {
  while (node.parent() && level>0) {
    node = node.parent();
    level--;
    if (node.type() != pugi::node_element) return false;
    if (node.name() == tagname) return true;
  }
  return false;
}

pugi::xml_node
DomUtil::lub(pugi::xml_node n1, pugi::xml_node n2) {
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
