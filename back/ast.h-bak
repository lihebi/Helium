#ifndef __AST_H__
#define __AST_H__
#include <pugixml.hpp>
#include <vector>
#include <map>
#include <iostream>

/**
 * This module is an overlay of AST, for srcml.
 */

namespace ast { // begin of namespace ast

  class Node;
  class Doc;
  Node* create_node(Doc* doc, pugi::xml_node node);

  class Node;



  /**
   * Doc is a pointer, because it should be init and destroyed.
   * Should not copy!

Doc is where host the pugi::xml_document.
It is possibly the same role as AST::Context in Clang, which is passed as a parameter for every AST class's constructor.
The role it takes is to host the AST tree itself during the analysis of the nodes.
Once the Doc is destroied(reseted), all nodes associated with it will become invalid.

   */
  class Doc {
  public:
    Doc();
    ~Doc();
    bool IsValid();
    Node* Root();
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

  typedef std::vector<Node*> NodeList;
  
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
    NK_Root,
    NK_NULL
  } NodeKind;

  /**
   * Node just maintain the xml_node itself, as well as the pointer to the ast_doc it corresponds to.
   * this is lightweight, can (and should) be copied.

Node should be designed as the structure. But for simplicity, or because of my lack of experience currently, I imbeded the parse of xml inside the class.
That means, every node's constructor receive a Doc*, and a pugi::xml_node, and it will check and parse the children nodes itself.
For example, IfNode will validate the tag of pugi::xml_node to be <if>, and get the <condition>,<then>,<else> tag and create the corresponding nodes for them, all inside the constructor of IfNode.

The Node is an abstract class. It should never be instantiated.
There may be some subclasses, like DeclNode, which can have subclasses like StmtDeclNode and ExprDeclNode.
Should I only allow instantiation of the most concrete classes?
Or should I design the Decl,Stmt,Expr all as interfaces?

   */
  class Node {
  public:
    Node() {}
    // Node(const Node&);
    virtual ~Node() {}
    bool IsValid() const;
    Node* Root();
    virtual NodeKind Kind() = 0;
    bool Contains(Node* node);
    /* travese */
    /**
     * TODO Since I'm doing AST modeling, should I include these XML traversal methods here?
     * Or I think I can just use a bunch of helper functions to traversal?
     */
    Node* PreviousSibling();
    Node* NextSibling();
    Node* Parent();
    // Node* FirstChild();
    // NodeList Children();
    /* text */
    std::string ToString() const; // for human read purpose only.
    std::string Text() const;
    std::string TextExceptComment() const; // TODO why I need to except comment? I'm extracting ID from code based on regexp matching, and that part should be changed by semantic way.
    int GetFirstLineNumber() const;

    /**
     * TODO These ast resolving method should really not here.
     * Put them into help functions.
     */
    // /* ast resolving now move here */
    // virtual std::vector<std::string> UndefinedVarNames() const {
    //   // TODO This function should be overwritten for several sub-classes
    //   return std::vector<std::string>();
    // }
    // // type as string ..
    // // or better return a Node?
    // // but anyhow can't return a Variable
    // virtual std::string ResolveVar(const std::string& name) const {
    //   // TODO this function should be overwritten for several sub-classes
    //   return "";
    // }
    
    operator bool() const {
      return IsValid();
    }

    /**
     * This is the feature that srcml offers, which is very nice.
     * Given the AST as xml, we can easily query, e.g. the children nodes of tag <xx>.
     */
    NodeList FindAll(NodeKind kind);
    
  protected:
    Doc *m_doc;
    pugi::xml_node m_node;
  };
  


  /*******************************
   ** Subclass Node
   *******************************/
  
  class RootNode : public Node {
  public:
    virtual NodeKind Kind() {return NK_Root;}
    RootNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~RootNode() {}
  };

  class StructNode;
  class UnionNode;
  
  class TypeNode : public Node {
  public:
    TypeNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~TypeNode() {}
    virtual NodeKind Kind() {return NK_Typedef;}
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
      m_node =node;
    }
    ~BlockNode() {}
    virtual NodeKind Kind() {return NK_Block;}
    NodeList Nodes() {
      NodeList nodes;
      for (pugi::xml_node n : m_node.children()) {
        if (n.type() == pugi::node_element) {
          nodes.push_back(create_node(m_doc, n));
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
    virtual NodeKind Kind() {return NK_Stmt;}
  };
  
  class DeclNode : public Node {
  public:
    DeclNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;

    }
    ~DeclNode() {}
    virtual NodeKind Kind() {return NK_Decl;}
    std::string Name() {
      return m_node.child_value("name");
    }
    std::string Init() {
      // FIXME need to find an example
      return m_node.child_value("init");
    }
    TypeNode* Type() {
      return new TypeNode(m_doc, m_node.child("type"));
    }
    std::map<std::string, std::string> GetTypeNameMapping() {
      std::map<std::string, std::string> m;
      std::string name = Name();
      std::string type = Type()->Name();
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
    virtual NodeKind Kind() {return NK_Expr;}
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
    IfNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~IfNode() {}
    virtual NodeKind Kind() {return NK_If;}
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
    virtual NodeKind Kind() {return NK_Case;}
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
    SwitchNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~SwitchNode() {}
    virtual NodeKind Kind() {return NK_Switch;}
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
    ForNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~ForNode() {}
    virtual NodeKind Kind() {return NK_For;}
    std::map<std::string, std::string> InitDecls() {
      std::map<std::string, std::string> result;
      Node* node = Init();
      if (node->Kind() == NK_Decl) {
        DeclNode* n = dynamic_cast<DeclNode*>(node);
        std::string name = n->Name();
        std::string type = n->Type()->Name();
        result[name] = type;
      }
      return result;
    }
    Node* Init() {
      pugi::xml_node node = m_node.child("control").child("init").child("expr");
      return create_node(m_doc, node);
    }
    ExprNode* Condition() {
      pugi::xml_node node = m_node.child("control").child("condition").child("expr");
      return new ExprNode(m_doc, node);
    }
    ExprNode* Incr() {
      pugi::xml_node node = m_node.child("control").child("incr").child("expr");
      return new ExprNode(m_doc, node);
    }
    BlockNode* Block() {
      pugi::xml_node node = m_node.child("block");
      return new BlockNode(m_doc, node);
    }
  };

  class WhileNode : public Node {
  public:
    WhileNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~WhileNode() {}
    virtual NodeKind Kind() {return NK_While;}
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
    DoNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~DoNode() {}
    virtual NodeKind Kind() {return NK_Do;}
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
    ~CallNode() {}
    virtual NodeKind Kind() {return NK_Call;}
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
    ParamNode(Doc* doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~ParamNode() {}
    virtual NodeKind Kind() {return NK_Param;}
    DeclNode Decl() {
      pugi::xml_node node = m_node.child("decl");
      return DeclNode(m_doc, node);
    }
  };
  
  typedef std::vector<ParamNode> ParamNodeList;

  class FunctionNode : public Node {
  public:
    FunctionNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~FunctionNode() {}
    virtual NodeKind Kind() {return NK_Function;}
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
    BlockNode* Block() {
      pugi::xml_node node = m_node.child("block");
      return new BlockNode(m_doc, node);
    }
    CallNode* GetCallNode() {
      std::string name = Name();
      pugi::xpath_node_set call_nodes = m_node.root().select_nodes("//call");
      for (auto it=call_nodes.begin();it!=call_nodes.end();it++) {
        if (strcmp(it->node().child_value("name"), name.c_str()) == 0) {
          return new CallNode(m_doc, it->node());
        }
      }
      return NULL;
    }
  };

  /* special */
  class BreakNode : public Node {
  public:
    BreakNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~BreakNode() {}
    virtual NodeKind Kind() {return NK_Break;}
  };
  class ContinueNode : public Node {
  public:
    ContinueNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~ContinueNode() {}
    virtual NodeKind Kind() {return NK_Continue;}
  };
  class ReturnNode : public Node {
  public:
    ReturnNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~ReturnNode() {}
    virtual NodeKind Kind() {return NK_Return;}
  };
  /* goto */
  class LabelNode : public Node {
  public:
    LabelNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~LabelNode() {}
    virtual NodeKind Kind() {return NK_Label;}
    std::string Name() {
      return m_node.child_value("name");
    }
  };
  class GotoNode : public Node {
  public:
    GotoNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~GotoNode() {}
    virtual NodeKind Kind() {return NK_Goto;}
    std::string Name() {
      return m_node.child_value("name");
    }
  };

  /* types */

  class TypedefNode : public Node {
  public:
    TypedefNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~TypedefNode() {}
    virtual NodeKind Kind() {return NK_Typedef;}
  };
  typedef std::vector<DeclStmtNode> DeclStmtNodeList;
  class StructNode : public Node {
  public:
    StructNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~StructNode() {}
    virtual NodeKind Kind() {return NK_Struct;}
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
    UnionNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~UnionNode() {}
    virtual NodeKind Kind() {return NK_Union;}
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
    EnumNode(Doc *doc, pugi::xml_node node) {
      m_doc = doc;
      m_node = node;
    }
    ~EnumNode() {}
    virtual NodeKind Kind() {return NK_Enum;}
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

  bool in_node(Node* node, NodeKind kind);
  // /**
  //  * node is a NK_Function node.
  //  * @return the Node <call> of the function.
  //  */
  // Node get_function_call(FunctionNode n);
  // Node get_function_call(std::string name, Node n);

  /**
   * true if one node in nodelist contains node
   */
  bool contains(NodeList nodes, Node* node);
} // end of namespace ast






#endif
