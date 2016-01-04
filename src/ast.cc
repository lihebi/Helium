#include "ast.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>

using namespace utils;

/*******************************
 ** Util functions
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
pugi::xml_node get_function_call(pugi::xml_node node) {
  const char *func_name = node.child_value("name");
  pugi::xpath_node_set call_nodes = node.root().select_nodes("//call");
  for (auto it=call_nodes.begin();it!=call_nodes.end();it++) {
    if (strcmp(it->node().child_value("name"), func_name) == 0) {
      return get_parent_ast_element(it->node());
    }
  }
  pugi::xml_node null_node;
  return null_node;
}

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
get_text_content_except(pugi::xml_node node, std::string name) {
  if (!node) return "";
  std::string text;
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      if (!node.attribute("helium-omit")) {
        if (strcmp(n.name(), name.c_str()) != 0) {
          text += get_text_content_except(n, name);
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
 * test if node is within <level> levels inside a <tagname>
 */
bool
in_node(pugi::xml_node node, std::string tagname, int level) {
  while (node.parent() && level>0) {
    node = node.parent();
    level--;
    if (node.type() != pugi::node_element) return false;
    if (node.name() == tagname) return true;
  }
  return false;
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


/*******************************
 ** AST functions
 *******************************/

/*******************************
 ** ast::Doc
 *******************************/

using namespace ast;

Doc::Doc() {}
Doc::~Doc() {
  if (m_doc) m_doc.reset();
  assert(!m_doc);
}

bool Doc::IsValid() {
  if (m_doc) return true;
  else return false;
}

Node Doc::Root() {
  return Node(this, m_doc.document_element());
}

/**
 * Init ast_doc* from file. Needs to be destroyed.
 */
void Doc::InitFromFile(const std::string &filename) {
  Doc *doc = (Doc*)malloc(sizeof(Doc));
  file2xml(filename, doc->m_doc);
}

/**
 * Init doc* from string. Need to be destroyed.
 */
void Doc::InitFromString(const std::string& code) {
  Doc *doc = (Doc*)malloc(sizeof(Doc));
  string2xml(code, doc->m_doc);
}

/*******************************
 ** Node
 *******************************/

Node::Node() {}
Node::Node(Doc* doc, pugi::xml_node node) : m_doc(doc), m_node(node) {}
Node::~Node() {}
bool Node::IsValid() const {
  if (m_doc && m_doc->IsValid()) return true;
  else return false;
}
Node Node::Root() {
  return Node(m_doc, m_node.root());
}

Node Node::PreviousSibling() {
  pugi::xml_node node = m_node.previous_sibling();
  if (node) {
    return Node(m_doc, node);
  } else {
    return Node(); // null node.
  }
}

Node Node::NextSibling() {
  pugi::xml_node node = m_node.next_sibling();
  if (node) {
    return Node(m_doc, node);
  } else {
    return Node();
  }
}


/*
 * return the first line markup of the node
 * -1 if no markup found
 */
int
Node::GetFirstLineNumber() const {
  pugi::xml_node node;
  try {
    node = m_node.select_node("//*[@pos::line]").node();
  } catch (pugi::xpath_exception) {
    // TODO
  }
  if (node) return atoi(node.attribute("pos::line").value());
  return -1;
}

/* traverse */

Node Node::Parent() {}

Node Node::FirstChild() {}

NodeList Node::Children() {}

std::string Node::Text() const {
  return get_text_content(m_node);
}
std::string Node::TextExceptComment() const {
  return get_text_content_except_tag(m_node, "comment");
}

bool Node::Contains(Node n) {
  return (lub(n.m_node, m_node) == m_node);
}

/**
 * test if node is within <level> levels inside a <tagname>
 */
// bool
// in_node(pugi::xml_node node, std::string tagname, int level) {
//   while (node.parent() && level>0) {
//     node = node.parent();
//     level--;
//     if (node.type() != pugi::node_element) return false;
//     if (node.name() == tagname) return true;
//   }
//   return false;
// }


bool in_node(Node node, NodeKind kind) {
  while ( (node = node.Parent()) ) {
    if (node.Type() == kind) return true;
  }
  return false;
}

/*******************************
 ** NodeList
 *******************************/

NodeList::NodeList() {}
NodeList::~NodeList() {}

// if node is a child noe of m_nodes. Caution: not node is in m_node!
bool NodeList::Contains(Node n) const {
  for (Node node : m_nodes) {
    if (node.Contains(n)) return true;
  }
  return false;
}
std::vector<Node> NodeList::Nodes() const {
  return m_nodes;
}
void NodeList::PushBack(Node node) {
  m_nodes.push_back(node);
}
void NodeList::PushBack(NodeList nodes) {
  for (Node node : nodes.Nodes()) {
    m_nodes.push_back(node);
  }
}

void NodeList::PushFront(Node node) {
  m_nodes.insert(m_nodes.begin(), node);
}

void NodeList::PushFront(NodeList nodes) {
  std::vector<Node> _nodes = nodes.Nodes();
  m_nodes.insert(m_nodes.begin(), _nodes.begin(), _nodes.end());
}
void NodeList::Clear() {
  m_nodes.clear();
}


/*******************************
 ** AST subclasses
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

std::vector<std::string> ExprNode::IDs() {
  std::vector<std::string> ids;
  for (pugi::xml_node n : m_node.children("name")) {
    std::string s = get_text_content(n);
    simplify_variable_name(s);
    ids.push_back(s);
  }
  return ids;
}

