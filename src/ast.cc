#include "ast.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>

using namespace utils;

/*******************************
 ** Dom Related Util functions
 *******************************/

bool is_valid_ast(const char* name);
pugi::xml_node get_previous_ast_element(pugi::xml_node node);
pugi::xml_node get_parent_ast_element(pugi::xml_node node);
pugi::xml_node get_function_call(pugi::xml_node node);

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
Node::Node(const Node&) {}
Node::~Node() {}
NodeKind Node::Type() {}
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



/*******************************
 ** helper functions
 *******************************/
bool ast::in_node(Node node, NodeKind kind) {
  while ( (node = node.Parent()) ) {
    if (node.Type() == kind) return true;
  }
  return false;
}

Node ast::get_function_call(Node n) {}

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

Node NodeList::Get(int index) const {
  if (index < 0 || (size_t)index >= m_nodes.size()) return Node();
  return m_nodes[index];
}

bool NodeList::Empty() const {
  return m_nodes.empty();
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


FunctionNode::FunctionNode() {
}
FunctionNode::~FunctionNode() {}
std::string FunctionNode::ReturnType() {}
std::map<std::string, std::string> FunctionNode::ParamList() {}
Node FunctionNode::Body() {}


std::map<std::string, std::string> ForNode::InitDecls() {}

std::map<std::string, std::string> DeclStmtNode::Decls() {}
