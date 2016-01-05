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

  // /**
  //  * A list of AST Nodes.
  //  */
  // class NodeList {
  // public:
  //   NodeList();
  //   ~NodeList();
  //   bool Contains(Node node) const;
  //   std::vector<Node> Nodes() const;
  //   void PushBack(Node node);
  //   void PushBack(NodeList nodes);
  //   void PushFront(Node node);
  //   void PushFront(NodeList nodes);
  //   void Clear();
  //   bool Empty() const;
  //   Node Get(int index) const;
  // private:
  //   std::vector<Node> m_nodes;
  // };

  typedef std::vector<Node> NodeList;
  
  typedef enum _NodeKind {
    NK_Function,
    NK_DeclStmt,
    NK_Decl,
    NK_ExprStmt,
    NK_Expr,
    NK_For,
    NK_Type,
    NK_Block,
    NK_Stmt,
    NK_If,
    NK_Case,
    NK_Default,
    NK_Switch,
    NK_While,
    NK_Do,
    NK_Call,
    NK_Param,
    NK_Break,
    NK_Continue,
    NK_Return,
    NK_Label,
    NK_Goto,
    NK_Typedef,
    NK_Struct,
    NK_Union,
    NK_Enum,
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
    NodeKind Kind();
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
    /* ast resolving now move here */
    virtual std::vector<std::string> UndefinedVarNames() const {
      // TODO This function should be overwritten for several sub-classes
      return std::vector<std::string>();
    }
    // type as string ..
    // or better return a Node?
    // but anyhow can't return a Variable
    virtual std::string ResolveVar(const std::string& name) const {
      // TODO this function should be overwritten for several sub-classes
      return "";
    }
    
    operator bool() const {
      return IsValid();
    }
    
  protected:
    Doc *m_doc;
    pugi::xml_node m_node;
    NodeKind m_kind;
  };
  


  /*******************************
   ** Subclass Node
   *******************************/

  class StructNode;
  class UnionNode;
  
  class TypeNode : public Node {
  public:
    TypeNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~TypeNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
    // std::string Modifier() {
    //   return "";
    // }
    // StructNode Struct();
    // UnionNode Union();
    // FIXME no enum??
  };
  
  /* general */
  class BlockNode : public Node {
  public:
    BlockNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~BlockNode() {}
    NodeList Nodes() {
      NodeList nodes;
      for (pugi::xml_node n : m_node.children()) {
        if (n.type() == pugi::node_element) {
          nodes.push_back(Node(m_doc, n));
        }
      }
      return nodes;
    }
  };

  class StmtNode : public Node {
  public:
    StmtNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~StmtNode() {}
  };
  
  class DeclNode : public Node {
  public:
    DeclNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~DeclNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
    std::string Init() {
      // FIXME need to find an example
      return m_node.child_value("init");
    }
    TypeNode Type() {
      return TypeNode(m_doc, m_node.child("type"));
    }
    std::map<std::string, std::string> GetTypeNameMapping() {
      std::map<std::string, std::string> m;
      std::string name = Name();
      std::string type = Type().Name();
      m[name] = type;
      return m;
    }
  };
  /* single stmt */
  class DeclStmtNode : public StmtNode {
  public:
    DeclStmtNode(Doc *doc, pugi::xml_node node) : StmtNode(doc, node) {}
    ~DeclStmtNode() {}
    std::map<std::string, std::string> Decls();
  };

  
  /* low */
  class ExprNode : public Node {
  public:
    ExprNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~ExprNode() {}
    std::vector<std::string> IDs();
  };
  class ExprStmtNode : public StmtNode {
  public:
    ExprStmtNode(Doc *doc, pugi::xml_node node) : StmtNode(doc, node) {}
    ~ExprStmtNode() {}
  };


  /* structures */
  class IfNode : public Node {
  public:
    IfNode() {}
    ~IfNode() {}
    ExprNode Condition() {
      pugi::xml_node node = m_node.child("condition").child("expr");
      return ExprNode(m_doc, node);
    }
    BlockNode Then() {
      pugi::xml_node node = m_node.child("then").child("block");
      return BlockNode(m_doc, node);
    }
    BlockNode Else() {
      pugi::xml_node node = m_node.child("else").child("block");
      return BlockNode(m_doc, node);
    }
  };

  typedef std::vector<ExprStmtNode> ExprStmtNodeList;
  typedef std::vector<StmtNode> StmtNodeList;
  class CaseNode : public Node {
  public:
    CaseNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~CaseNode() {}
    ExprNode Expr() {
      pugi::xml_node node = m_node.child("expr");
      return ExprNode(m_doc, node);
    }
    /**
     * The stmt can be either ExprStmt or DeclStmt!!
     */
    StmtNodeList Stmts() {
      StmtNodeList nodes;
      pugi::xml_node node = m_node;
      // a case may or may not have a block.
      // FIXME have two blocks?
      if (node.child("block")) {
        node = node.child("block");
      }
      for (pugi::xml_node n : node.children()) {
        if (strcmp(n.name(), "decl_stmt") == 0 || strcmp(n.name(), "expr_stmt") == 0) {
          nodes.push_back(StmtNode(m_doc, n));
        }
      }
      return nodes;
    }
  };
  class DefaultNode : public CaseNode {
  public:
    // FIXME ????? HEBI
    DefaultNode(Doc* doc, pugi::xml_node node) : CaseNode(doc, node) {}
    ~DefaultNode() {}
    // inherit the Stmts() function from CaseNode
  };
  
  typedef std::vector<CaseNode> CaseNodeList;

  class SwitchNode : public Node {
  public:
    SwitchNode() {}
    ~SwitchNode() {}
    ExprNode Condition() {
      pugi::xml_node node = m_node.child("condition").child("expr");
      return ExprNode(m_doc, node);
    }
    CaseNodeList Cases() {
      CaseNodeList nodes;
      for (pugi::xml_node n : m_node.child("block").children("case")) {
        nodes.push_back(CaseNode(m_doc, n));
      }
      return nodes;
    }
    DefaultNode Default() {
      pugi::xml_node node = m_node.child("block").child("default");
      return DefaultNode(m_doc, node);
    }
  };


  /* loop */
  class ForNode : public Node {
  public:
    ForNode() {}
    ~ForNode() {}
    std::map<std::string, std::string> InitDecls() {
      std::map<std::string, std::string> result;
      Node node = Init();
      if (node.Kind() == NK_Decl) {
        DeclNode n = dynamic_cast<DeclNode&>(node);
        std::string name = n.Name();
        std::string type = n.Type().Name();
        result[name] = type;
      }
      return result;
    }
    Node Init() {
      pugi::xml_node node = m_node.child("control").child("init").child("expr");
      return Node(m_doc, node);
    }
    ExprNode Condition() {
      pugi::xml_node node = m_node.child("control").child("condition").child("expr");
      return ExprNode(m_doc, node);
    }
    ExprNode Incr() {
      pugi::xml_node node = m_node.child("control").child("incr").child("expr");
      return ExprNode(m_doc, node);
    }
    BlockNode Block() {
      pugi::xml_node node = m_node.child("block");
      return BlockNode(m_doc, node);
    }
  };

  class WhileNode : public Node {
  public:
    WhileNode() {}
    ~WhileNode() {}
    ExprNode Condition() {
      pugi::xml_node node = m_node.child("condition").child("expr");
      return ExprNode(m_doc, node);
    }
    BlockNode Block() {
      pugi::xml_node node = m_node.child("block");
      return BlockNode(m_doc, node);
    }
  };

  class DoNode : public Node {
  public:
    DoNode() {}
    ~DoNode() {}
    BlockNode Block() {
      pugi::xml_node node = m_node.child("block");
      return BlockNode(m_doc, node);
    }
    ExprNode Condition() {
      pugi::xml_node node = m_node.child("condition").child("expr");
      return ExprNode(m_doc, node);
    }
  };

  
  /* function */
  typedef std::vector<ExprNode> ExprNodeList;
  class CallNode : public Node {
  public:
    CallNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    CallNode() {}
    ~CallNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
    ExprNodeList Arguments() {
      ExprNodeList nodes;
      for (pugi::xml_node n : m_node.child("argument_list").children("argument")) {
        nodes.push_back(ExprNode(m_doc, n.child("expr")));
      }
      return nodes;
    }
  };
  class ParamNode : public Node {
  public:
    /**
     * TODO param is here, instead of raw DeclNode, is because we want to model int foo(a) int a; {}
     */
    ParamNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~ParamNode() {}
    DeclNode Decl() {
      pugi::xml_node node = m_node.child("decl");
      return DeclNode(m_doc, node);
    }
  };
  
  typedef std::vector<ParamNode> ParamNodeList;

  class FunctionNode : public Node {
  public:
    FunctionNode() {}
    ~FunctionNode() {}
    std::string Type() {
      return m_node.child("type").child_value("name");
    }
    std::string Name() {
      return m_node.child_value("name");
    }
    // FIXME for legacy C function definition syntax, after paramlist, there're some decl_stmt.
    ParamNodeList Params() {
      ParamNodeList nodes;
      for (pugi::xml_node n : m_node.child("parameter_list").children("param")) {
        nodes.push_back(ParamNode(m_doc, n));
      }
      return nodes;
    }
    BlockNode Block() {
      pugi::xml_node node = m_node.child("block");
      return BlockNode(m_doc, node);
    }
    CallNode GetCallNode() {
      std::string name = Name();
      pugi::xpath_node_set call_nodes = m_node.root().select_nodes("//call");
      for (auto it=call_nodes.begin();it!=call_nodes.end();it++) {
        if (strcmp(it->node().child_value("name"), name.c_str()) == 0) {
          return (CallNode(m_doc, it->node()));
        }
      }
      return CallNode();
    }
  };

  /* special */
  class BreakNode : public Node {
  public:
    BreakNode() {}
    ~BreakNode() {}
  };
  class ContinueNode : public Node {
  public:
    ContinueNode() {}
    ~ContinueNode() {}
  };
  class ReturnNode : public Node {
  public:
    ReturnNode() {}
    ~ReturnNode() {}
  };
  /* goto */
  class LabelNode : public Node {
  public:
    LabelNode() {}
    ~LabelNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
  };
  class GotoNode : public Node {
  public:
    GotoNode() {}
    ~GotoNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
  };

  /* types */

  class TypedefNode : public Node {
  public:
    TypedefNode() {}
    ~TypedefNode() {}
  };
  typedef std::vector<DeclStmtNode> DeclStmtNodeList;
  class StructNode : public Node {
  public:
    StructNode() {}
    ~StructNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
    DeclStmtNodeList Fields() {
      DeclStmtNodeList nodes;
      for (pugi::xml_node n : m_node.child("block").children("decl_stmt")) {
        nodes.push_back(DeclStmtNode(m_doc, n));
      }
      return nodes;
    }
  };
  class UnionNode : public Node {
  public:
    UnionNode() {}
    ~UnionNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
    DeclStmtNodeList Fields() {
      // copied from StructNode
      DeclStmtNodeList nodes;
      for (pugi::xml_node n : m_node.child("block").children("decl_stmt")) {
        nodes.push_back(DeclStmtNode(m_doc, n));
      }
      return nodes;
    }
  };
  
  class EnumNode : public Node {
  public:
    EnumNode() {}
    ~EnumNode() {}
    std::string Name() {
      return m_node.child_value("name");
    }
    DeclStmtNodeList Fields() {
      // copied from StructNode
      // These decl_stmt do not have a <type>
      DeclStmtNodeList nodes;
      for (pugi::xml_node n : m_node.child("block").children("decl_stmt")) {
        nodes.push_back(DeclStmtNode(m_doc, n));
      }
      return nodes;
    }
  };


  
  /*******************************
   ** Helper function
   *******************************/

  bool in_node(Node node, NodeKind kind);
  // /**
  //  * node is a NK_Function node.
  //  * @return the Node <call> of the function.
  //  */
  // Node get_function_call(FunctionNode n);
  // Node get_function_call(std::string name, Node n);

  /**
   * true if one node in nodelist contains node
   */
  bool contains(NodeList nodes, Node node);


    
    

  
} // end of namespace ast



#endif
