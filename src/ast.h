#ifndef __AST_H__
#define __AST_H__
#include <pugixml.hpp>
#include <vector>
#include <map>

/**
 * This module is an overlay of AST, for srcml.
 */

namespace ast { // begin of namespace ast

  class Node;



  /**
   * Doc is a pointer, because it should be init and destroyed.
   * Should not copy!
   */
  class Doc {
  public:
    Doc();
    ~Doc();
    bool IsValid();
    Node Root();
    void InitFromString(const std::string &code);
    void InitFromFile(const std::string &filename);
  private:
    Doc(const Doc&) {} // cannot be copied
    pugi::xml_document m_doc;
  };

  /**
   * A list of AST Nodes.
   */
  class NodeList {
  public:
    NodeList();
    ~NodeList();
    bool Contains(Node node) const;
    std::vector<Node> Nodes() const;
    void PushBack(Node node);
    void PushBack(NodeList nodes);
    void PushFront(Node node);
    void PushFront(NodeList nodes);
    void Clear();
    bool Empty() const;
    Node Get(int index) const;
  private:
    std::vector<Node> m_nodes;
  };

  typedef enum _NodeKind {
    NK_Function,
    NK_DeclStmt,
    NK_Expr,
    NK_For,
    NK_NULL
  } NodeKind;

  /**
   * Node just maintain the xml_node itself, as well as the pointer to the ast_doc it corresponds to.
   * this is lightweight, can (and should) be copied.
   */
  class Node {
  public:
    Node();
    Node(Doc* doc, pugi::xml_node node);
    Node(const Node&);
    virtual ~Node();
    bool IsValid() const;
    Node Root();
    NodeKind Type();
    bool Contains(Node node);
    /* travese */
    Node PreviousSibling();
    Node NextSibling();
    Node Parent();
    Node FirstChild();
    NodeList Children();
    /* text */
    std::string Text() const;
    std::string TextExceptComment() const;
    int GetFirstLineNumber() const;
    
    operator bool() const {
      return IsValid();
    }

  protected:
    Doc *m_doc;
    pugi::xml_node m_node;
  };

  bool in_node(Node node, NodeKind kind);
  /**
   * node is a NK_Function node.
   * @return the Node <call> of the function.
   */
  Node get_function_call(Node n);

  class FunctionNode : public Node {
  public:
    FunctionNode();
    ~FunctionNode();
    std::string ReturnType();
    std::map<std::string, std::string> ParamList(); // {name: type, name: type}
    Node Body();
  private:
  };

  class DeclStmtNode : public Node {
  public:
    std::map<std::string, std::string> Decls();
  };

  class ForNode : public Node {
  public:
    std::map<std::string, std::string> InitDecls();
  };
  
  class ExprNode : public Node {
  public:
    std::vector<std::string> IDs();
  };

  
} // end of namespace ast
#endif
