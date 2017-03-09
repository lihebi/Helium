#include "ast.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>
#include <iostream>

using namespace utils;

/*******************************
 ** Dom Related Util functions
 *******************************/

bool is_valid_ast(const char* name);
bool is_valid_ast(pugi::xml_node node);
pugi::xml_node get_previous_ast_element(pugi::xml_node node);
pugi::xml_node get_parent_ast_element(pugi::xml_node node);
// pugi::xml_node get_function_call(pugi::xml_node node);

std::string get_text_content(pugi::xml_node node);
std::string get_text_content_except_tag(pugi::xml_node, std::string name);

pugi::xml_node lub(pugi::xml_node n1, pugi::xml_node n2);
bool dom_in_node(pugi::xml_node node, std::string tagname, int level);

/*******************************
 ** ast::Doc
 *******************************/

using namespace ast;

Doc::Doc() {}
Doc::~Doc() {
  if (m_doc) m_doc.reset();
  assert(!m_doc.document_element());
}

bool Doc::IsValid() {
  if (m_doc && m_doc.document_element()) return true;
  else return false;
}

/**
 * Return the root node.
 * CAUTION This is not the root of ast, but the root of the xml document.
 * It is not practical to get the root of ast, for example, a c file has many ASTs
 */
Node* Doc::Root() {
  /**
   * 
#include<xxx.h>

#define

func() {
}

func() {
}

So it just matters about the function node.
Thus even if we get the funciton node, we cannot query #define value.
So, no need to get the actual AST root.
Just use the root of xml, and query like FindNodes(NK_Function) to get the function AST root.
   */
  // m_doc.document_element().print(std::cout);
  Node* node = create_node(this, m_doc.document_element());
  return node;
}

/**
 * Init ast_doc* from file. Needs to be destroyed.
 */
void Doc::InitFromFile(const std::string &filename) {
  // Doc *doc = (Doc*)malloc(sizeof(Doc));
  file2xml(filename, m_doc);
}

/**
 * Init doc* from string. Need to be destroyed.
 */
void Doc::InitFromString(const std::string& code) {
  // Doc *doc = (Doc*)malloc(sizeof(Doc));
  string2xml(code, m_doc);
}

/*******************************
 ** Node
 *******************************/

Node* Node::PreviousSibling() {
  pugi::xml_node n = m_node;
  while ( (n = n.previous_sibling()) ) {
    if (is_valid_ast(n)) {
      return create_node(m_doc, n);
    }
  }
  return NULL;
}
Node* Node::NextSibling() {
  pugi::xml_node n = m_node;
  while ( (n = n.previous_sibling())) {
    if (is_valid_ast(n)) {
      return create_node(m_doc, n);
    }
  }
  return NULL;
}
Node* Node::Parent() {
  pugi::xml_node n = m_node;
  while ( (n = n.parent())) {
    if (is_valid_ast(n)) {
      return create_node(m_doc, n);
    }
  }
  return NULL;
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

std::string Node::Text() const {
  return get_text_content(m_node);
}
std::string Node::TextExceptComment() const {
  return get_text_content_except_tag(m_node, "comment");
}

bool Node::Contains(Node* n) {
  return (lub(n->m_node, m_node) == m_node);
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

NodeList Node::FindAll(NodeKind kind) {
  std::cout <<"finding all.."  << "\n";
  NodeList result;
  std::string tag;
  switch (kind) {
  case NK_Function: {
    tag = "//function";
  }
  default: {
    ;
  }
  }
  pugi::xpath_node_set nodes = m_node.select_nodes(tag.c_str());
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    FunctionNode *function = static_cast<FunctionNode*>(create_node(m_doc, it->node()));
    function->Kind();
    result.push_back(function);
  }
  return result;
}



/*******************************
 ** helper functions
 *******************************/
bool ast::in_node(Node* node, NodeKind kind) {
  while ( (node = node->Parent()) ) {
    if (node->Kind() == kind) return true;
  }
  return false;
}


bool ast::contains(NodeList nodes, Node* node) {
  for (Node* n : nodes) {
    if (n->Contains(node)) return true;
  }
  return false;
}

// /*******************************
//  ** NodeList
//  *******************************/

// NodeList::NodeList() {}
// NodeList::~NodeList() {}

// // if node is a child noe of m_nodes. Caution: not node is in m_node!
// bool NodeList::Contains(Node n) const {
//   for (Node node : m_nodes) {
//     if (node.Contains(n)) return true;
//   }
//   return false;
// }
// std::vector<Node> NodeList::Nodes() const {
//   return m_nodes;
// }
// void NodeList::PushBack(Node node) {
//   m_nodes.push_back(node);
// }
// void NodeList::PushBack(NodeList nodes) {
//   for (Node node : nodes.Nodes()) {
//     m_nodes.push_back(node);
//   }
// }

// void NodeList::PushFront(Node node) {
//   m_nodes.insert(m_nodes.begin(), node);
// }

// void NodeList::PushFront(NodeList nodes) {
//   std::vector<Node> _nodes = nodes.Nodes();
//   m_nodes.insert(m_nodes.begin(), _nodes.begin(), _nodes.end());
// }
// void NodeList::Clear() {
//   m_nodes.clear();
// }

// Node NodeList::Get(int index) const {
//   if (index < 0 || (size_t)index >= m_nodes.size()) return Node();
//   return m_nodes[index];
// }

// bool NodeList::Empty() const {
//   return m_nodes.empty();
// }



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


std::map<std::string, std::string> DeclStmtNode::Decls() {
  std::map<std::string, std::string> result;
  pugi::xml_node node = m_node.child("decl");
  std::string type_str = get_text_content(node.child("type"));
  std::string name_str = get_text_content(node.child("name"));
  if (name_str.find('[') != std::string::npos) {
    type_str += name_str.substr(name_str.find('['));
  }
  name_str = name_str.substr(0, name_str.find('['));
  result[name_str] = type_str;
  return result;
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


Node* ast::create_node(Doc* doc, pugi::xml_node node) {
  const char * name = node.name();
  if (strcmp(name, "function")) return new FunctionNode(doc, node);
  if (strcmp(name, "if")) return new IfNode(doc, node);
  return NULL;
}
