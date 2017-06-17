#ifndef VISITOR_H
#define VISITOR_H

// #include "helium/parser/ast_v2.h"

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <boost/algorithm/string/join.hpp>

#include "helium/parser/source_location.h"
#include "helium/utils/graph.h"

namespace v2 {
  class ASTContext;
  
  class ASTNodeBase;
  class TokenNode;
  class TranslationUnitDecl;
  class FunctionDecl;
  class Decl;
  class Stmt;
  class DeclStmt;
  class ExprStmt;
  class CompoundStmt;
  class ForStmt;
  class WhileStmt;
  class DoStmt;
  class BreakStmt;
  class ContinueStmt;
  class ReturnStmt;
  class IfStmt;
  class SwitchStmt;
  class SwitchCase;
  class CaseStmt;
  class DefaultStmt;
  class Expr;
}


/**
 * \defgroup visitor
 * \ingroup parser
 * Visitors for AST
 */

/**
 * \ingroup visitor
 *
 * visitor interface
 */
class Visitor {
public:
  Visitor() {}
  virtual ~Visitor() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);
};


class SimplePreorderVisitor : public Visitor {
public:
  SimplePreorderVisitor() {}
  virtual ~SimplePreorderVisitor() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);
  std::vector<v2::ASTNodeBase*> getNodes() {
    return Nodes;
  }
private:
  std::vector<v2::ASTNodeBase*> Nodes;
};

/**
 * \ingroup visitor
 *  use this to replace the dump method
 */
class Printer : public  Visitor {
public:
  Printer() {}
  ~Printer() {}


  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  std::string prettyPrint(std::string aststr);

  std::string getString() {
    return prettyPrint(oss.str());
  }
private:
  void pre(v2::ASTNodeBase *node);
  void post();
  std::ostringstream oss;
};


class LevelVisitor : public Visitor {
public:
  LevelVisitor() {}
  virtual ~LevelVisitor() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  std::map<v2::ASTNodeBase*, int> getLevels() {return Levels;}
  int getLevel(v2::ASTNodeBase *node);
  v2::ASTNodeBase *getLowestLevelNode(std::set<v2::ASTNodeBase*> nodes);
private:
  void pre(v2::ASTNodeBase* node) {Levels[node]=lvl; lvl++;}
  void post() {lvl--;}
  std::map<v2::ASTNodeBase*, int> Levels;
  int lvl=0;
};

namespace v2 {
  class SymbolTable {
  public:
    SymbolTable() {}
    ~SymbolTable() {}

    void pushScope() {
      Stack.push_back({});
    }
    void popScope() {
      Stack.pop_back();
    }

    /**
     * id is declared in node
     */
    void add(std::string id, v2::ASTNodeBase *node) {
      newlyAdded.clear();
      newlyAdded.insert(id);
      Stack.back()[id] = node;
    }
    void add(std::set<std::string> ids, v2::ASTNodeBase *node) {
      newlyAdded = ids;
      for (const std::string&id : ids) {
        add(id, node);
      }
    }
    v2::ASTNodeBase* get(std::string id) {
      for (int i=Stack.size()-1;i>=0;i--) {
        if (Stack[i].count(id) == 1) return Stack[i][id];
      }
      return nullptr;
    }
    std::set<std::string> getNewlyAdded() {return newlyAdded;}
    /**
     * get all visible ones
     */
    std::map<std::string, v2::ASTNodeBase*> getAll() {
      std::map<std::string, v2::ASTNodeBase*> ret;
      for (std::map<std::string, v2::ASTNodeBase*> v : Stack) {
        for (auto m : v) {
          ret[m.first] = m.second;
        }
      }
      return ret;
    }
  private:
    std::vector<std::map<std::string, v2::ASTNodeBase*> > Stack;
    // this is a hack
    // i notice in symboltablebuilder, each node adds exactly once (may be using vars)
    // so newly added can record the variables the current node defines
    std::set<std::string> newlyAdded;
  };
};

/**
 * The symbol table should be easy to access for each node.
 * So I might consider to
 * - create the symbol table right after I create the AST
 * - store the symbol table with the AST nodes
 */
class SymbolTableBuilder : public Visitor {
public:
  SymbolTableBuilder() {}
  virtual ~SymbolTableBuilder() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);
  std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > getUse2DefMap() {return Use2DefMap;}
  void dump(std::ostream &os);
  std::map<v2::ASTNodeBase*, v2::SymbolTable> getPersistTables() {return PersistTables;}
private:
  void insertDefUse(v2::ASTNodeBase *use);
  // this will be empty after traversal
  v2::SymbolTable Table;
  // this is the result
  std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > Use2DefMap;
  // record table at each node, for later usage
  std::map<v2::ASTNodeBase*, v2::SymbolTable> PersistTables;
};

/**
 * Token here means the leaf-node of AST, so it is abstract token.
 * The granularity depends on the parser precision.
 */
class TokenVisitor : public Visitor {
public:
  TokenVisitor() {}
  ~TokenVisitor() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  std::vector<v2::ASTNodeBase*> getTokens() {return Tokens;}
  std::map<v2::ASTNodeBase*,int> getIdMap() {return IdMap;}
  int getId(v2::ASTNodeBase *node) {
    if (IdMap.count(node) == 1) return IdMap[node];
    return -1;
  }
private:
  int id = 0;
  std::map<v2::ASTNodeBase*,int> IdMap; // ID start from 0
  std::vector<v2::ASTNodeBase*> Tokens; // this is actually ID->Node implemented in vector
};

/**
 * Build a reverse index from children node to its parent
 */
class ParentIndexer : public Visitor {
public:
  ParentIndexer() {}
  ~ParentIndexer() {}

  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  std::map<v2::ASTNodeBase*, v2::ASTNodeBase*> getParentMap() {return ParentMap;}
  std::vector<v2::ASTNodeBase*> getChildren(v2::ASTNodeBase* parent) {
    if (ChildrenMap.count(parent) == 1) {
      return ChildrenMap[parent];
    }
    return {};
  }

  v2::ASTNodeBase *getParent(v2::ASTNodeBase *node) {
    if (ParentMap.count(node) == 1) return ParentMap[node];
    return nullptr;
  }
private:
  void pre(v2::ASTNodeBase* node);
  void post();
  std::map<v2::ASTNodeBase*, v2::ASTNodeBase*> ParentMap;
  std::map<v2::ASTNodeBase*, std::vector<v2::ASTNodeBase*> > ChildrenMap;

  std::vector<v2::ASTNodeBase*> Stack;
};





/**
 * Compute the distribution of tokens, e.g. inside which if, which function
 *
 * This is also going to be bi-direction.
 *
 * map<Node, set> maps a node to all the tokens it contains. Notice that this node should be the representative nodes, like if, switch, loop.
 * set<Node> if_nodes;
 * set<Node> switch_nodes;
 * set<Node> for_nodes;
 * set<Node> do_nodes;
 * set<Node> while_nodes;
 * set<Node> func_nodes;
 */
class Distributor : public Visitor {
public:
  Distributor() {}
  ~Distributor() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  void pre(v2::ASTNodeBase *node);
  void post();

  /**
   * get how many functions enclosing these nodes
   * Note these nodes may not belong to this AST
   */
  int getDistFunc(std::set<v2::ASTNodeBase*> sel) {
    return getDist(sel, func_nodes);
  }
  std::set<v2::ASTNodeBase*> getDistFuncNodes(std::set<v2::ASTNodeBase*> sel) {
    return getDistNodes(sel, func_nodes);
  }
  int getDistIf(std::set<v2::ASTNodeBase*> sel) {
    return getDist(sel, if_nodes);
  }
  int getDistLoop(std::set<v2::ASTNodeBase*> sel) {
    std::set<v2::ASTNodeBase*> loop_nodes;
    loop_nodes.insert(for_nodes.begin(), for_nodes.end());
    loop_nodes.insert(while_nodes.begin(), while_nodes.end());
    loop_nodes.insert(do_nodes.begin(), do_nodes.end());
    return getDist(sel, loop_nodes);
  }
  int getDistSwitch(std::set<v2::ASTNodeBase*> sel) {
    return getDist(sel, switch_nodes);
  }

  /**
   * Generic function for get distribution
   */
  int getDist(std::set<v2::ASTNodeBase*> sel, std::set<v2::ASTNodeBase*> nodes) {
    int ret=0;
    for (auto *node : nodes) {
      for (auto *s : sel) {
        if (ContainMap[node].count(s) == 1) {
          ret++;
          break;
        }
      }
    }
    return ret;
  }
  std::set<v2::ASTNodeBase*> getDistNodes(std::set<v2::ASTNodeBase*> sel, std::set<v2::ASTNodeBase*> nodes) {
    std::set<v2::ASTNodeBase*> ret;
    for (auto *node : nodes) {
      for (auto *s : sel) {
        if (ContainMap[node].count(s) == 1) {
          ret.insert(node);
          break;
        }
      }
    }
    return ret;
  }

  void dump(std::ostream &os);

  std::set<v2::ASTNodeBase*> getIfNodes() {return if_nodes;}
  std::set<v2::ASTNodeBase*> getSwitchNodes() {return switch_nodes;}
  std::set<v2::ASTNodeBase*> getForNodes() {return for_nodes;}
  std::set<v2::ASTNodeBase*> getDoNodes() {return do_nodes;}
  std::set<v2::ASTNodeBase*> getWhileNodes() {return while_nodes;}
  std::set<v2::ASTNodeBase*> getFuncNodes() {return func_nodes;}
  
  
private:
  std::map<v2::ASTNodeBase*, std::set<v2::ASTNodeBase*> > ContainMap;
  std::set<v2::ASTNodeBase*> if_nodes;
  std::set<v2::ASTNodeBase*> switch_nodes;
  std::set<v2::ASTNodeBase*> for_nodes;
  std::set<v2::ASTNodeBase*> do_nodes;
  std::set<v2::ASTNodeBase*> while_nodes;
  std::set<v2::ASTNodeBase*> func_nodes;

  std::vector<v2::ASTNodeBase*> Stack;
};





/**
 * Code Generator
 */
class Generator : public Visitor {
public:
  Generator() {}
  virtual ~Generator() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);


  void setSelection(std::set<v2::ASTNodeBase*> sel) {
    selection = sel;
  }

  void setInputVarNodes(std::set<v2::ASTNodeBase*> nodes) {
    InputVarNodes = nodes;
  }

  std::string getProgram() {
    return Prog;
  }
  void adjustReturn(bool b) {
    AdjustReturn = b;
  }

  void setOutputInstrument(std::map<std::string, v2::ASTNodeBase*> toinstrument, v2::ASTNodeBase *last, v2::SymbolTable symtbl) {
    OutputInstrument = toinstrument;
    OutputPosition = last;
    OutputPositionSymtbl = symtbl;
  }
  void outputInstrument(v2::ASTNodeBase *node);
  std::vector<int> getIOSummary() {
    return {sum_output_var, sum_used_output_var, sum_input_var, sum_used_input_var};
  }
private:
  std::string Prog;
  std::set<v2::ASTNodeBase*> selection;
  bool AdjustReturn = false;
  std::set<v2::ASTNodeBase*> InputVarNodes;
  std::map<std::string, v2::ASTNodeBase*> OutputInstrument;
  v2::ASTNodeBase *OutputPosition = nullptr;
  v2::SymbolTable OutputPositionSymtbl;
  // summary about output instrumentation
  int sum_output_var=0;
  int sum_used_output_var=0;
  // summary about input instrumentation
  int sum_input_var=0;
  int sum_used_input_var=0;
};


/**
 * This is simple AST matcher
 * It is used in testing for locating the node I want.
 * I'm going to visit each node, and get the full path of them
 * Then just simply match.
 *
 * As input,
 * Specify the full path of the node you want
 * E.g. /DeclarationUnitDecl/Function/...
 * 1. n-th: if the path match multiple nodes, return all of them. The order is TODO
 * 2. return nullptr if no such node
 *
 * A second approach to match a node is through location and node name
 * e.g. line 5, column 8, IfStmt
 */
class Matcher : public Visitor {
public:
  Matcher() {}
  virtual ~Matcher() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  std::vector<v2::ASTNodeBase*> match(std::string name) {
    if (PathMap.count(name) == 1) {
      return PathMap[name];
    }
    return {};
  }
  int size() {return PathMap.size();}
  void dump(std::ostream &os) {
    for (auto m : PathMap) {
      os << m.first << m.second.size() << "\n";
    }
  }

  /**
   * use line, don't care about column
   * Must start on that line
   */
  v2::ASTNodeBase* getNodeByLoc(std::string name, int line);
  /**
   * nth: 0 means first, 1 means second
   */
  v2::ASTNodeBase* getNodeByLoc(std::string name, int line, int nth);
  /**
   * Use both line and column
   */
  v2::ASTNodeBase* getNodeByLoc(std::string name, SourceLocation loc);
  v2::ASTNodeBase* getNodeByLoc(std::string name, SourceLocation loc, int nth);

  /**
   * Dump:
   * - path to node map
   * - all nodes
   */
  // void dump(std::ostream &os);

private:
  void pre(v2::ASTNodeBase* node);
  void post();
  std::string currentName() {
    return boost::algorithm::join(Stack, "/");
  }
  std::vector<std::string> Stack;
  std::map<std::string, std::vector<v2::ASTNodeBase*> > PathMap;

  std::vector<v2::ASTNodeBase*> Nodes;
};


struct InstrumentPoint {
  // true for before
  // false for after
  bool before = true;
  v2::ASTNodeBase *node = nullptr;
};

/**
 * Decide the instrumentation point before and after each node.
 */
class InstrumentPointVisitor : public Visitor {
public:
  InstrumentPointVisitor() {}
  ~InstrumentPointVisitor() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);
private:
  std::map<v2::ASTNodeBase*, InstrumentPoint> After;
};



namespace v2 {

  class CFGNode {
  public:
    CFGNode(ASTNodeBase*node) {
      astnode = node;
    }
    CFGNode(std::string dummy) {
      this->dummy = dummy;
    }
    ~CFGNode() {}
    std::string getLabel();
  private:
    ASTNodeBase *astnode=nullptr;
    std::string dummy;
  };
  class CFG {
  public:
    CFG() {}
    ~CFG() {}

    // add nodes
    void addNode(CFGNode *node) {
      graph.addNode(node);
    }
    void addNodes(std::set<CFGNode*> nodes) {
      for (CFGNode *node : nodes) {
        graph.addNode(node);
      }
    }
    /**
     * Merge
     */
    void connect(CFG *cfg) {
      this->graph.merge(cfg->graph);
      for (CFGNode *from : this->outs) {
        for (CFGNode *to : cfg->ins) {
          this->graph.addEdge(from, to);
        }
      }
      this->outs = cfg->outs;
    }
    /**
     * add node and connect all exits to it (it will be the out)
     */
    void connect(CFGNode *node) {
      this->graph.addNode(node);
      for (CFGNode *from : this->outs) {
        this->graph.addEdge(from, node);
      }
      this->outs.clear();
      this->outs.insert(node);
    }
    void mergeBranch(CFG *cfg, CFGNode* node, bool b) {
      for (CFGNode *to : cfg->ins) {
        this->graph.addEdge(node, to, (b?"true":"false"));
      }
      this->outs.insert(cfg->outs.begin(), cfg->outs.end());
    }

    void mergeCase(CFG *cfg, CFGNode *node, std::string case_label) {
      for (CFGNode *to : cfg->ins) {
        this->graph.addEdge(node, to, case_label);
      }
      this->outs.insert(cfg->outs.begin(), cfg->outs.end());
    }
    
    void addEdge(CFGNode *from, CFGNode *to) {
      graph.addEdge(from, to);
    }
    /**
     * remove the out edge of node
     * Used to handle break, continue, return
     */
    void removeOutEdge(CFGNode *node) {
      graph.removeOutEdge(node);
    }
    std::string visualize();
    std::string visualizeAgg();

    /**
     * I still need to set in and out manually because, return xx; a=b;
     * When I remove the return out edge, a=b; will be the input.
     * There're several such bugs
     */
    void addIn(CFGNode *node) {
      ins.insert(node);
    }
    void addOut(CFGNode *node) {
      outs.insert(node);
    }
    void clearIn(CFGNode *node) {
      ins.clear();
    }
    void clearOut(CFGNode *node) {
      outs.clear();
    }
    // friend class CFGBuilder;
    // friend is not working, probably due to namespace
    // i'm making the fields public for the time
    hebigraph::Graph<CFGNode*> graph;
    std::set<CFGNode*> ins;
    std::set<CFGNode*> outs;
  private:
  };
};

class CFGBuilder : public Visitor {
public:
  CFGBuilder() {}
  ~CFGBuilder() {}
  // high level
  virtual void visit(v2::TokenNode *node);
  virtual void visit(v2::TranslationUnitDecl *node);
  virtual void visit(v2::FunctionDecl *node);
  virtual void visit(v2::CompoundStmt *node);
  // condition
  virtual void visit(v2::IfStmt *node);
  virtual void visit(v2::SwitchStmt *node);
  virtual void visit(v2::CaseStmt *node);
  virtual void visit(v2::DefaultStmt *node);
  // loop
  virtual void visit(v2::ForStmt *node);
  virtual void visit(v2::WhileStmt *node);
  virtual void visit(v2::DoStmt *node);
  // single
  virtual void visit(v2::BreakStmt *node);
  virtual void visit(v2::ContinueStmt *node);
  virtual void visit(v2::ReturnStmt *node);
  // expr stmt
  virtual void visit(v2::Expr *node);
  virtual void visit(v2::DeclStmt *node);
  virtual void visit(v2::ExprStmt *node);

  void pre(v2::ASTNodeBase* node);

  /**
   * CFG for the top level passed in
   */
  v2::CFG* getCFG() {return cur_cfg;}

  v2::CFG *getCFGByNode(v2::ASTNodeBase *node) {
    if (Node2CFG.count(node)) {
      return Node2CFG[node];
    }
    return nullptr;
  }

  v2::CFG* getInnerCFG(v2::ASTNodeBase* node) {
    assert(node);
    assert(Node2CFG.count(node) == 1);
    return Node2CFG[node];
  }
  void addInnerCFG(v2::ASTNodeBase* node, v2::CFG *cfg) {
    assert(cfg);
    assert(node);
    Node2CFG[node]=cfg;
    // record current, for the top level output
    cur_cfg = cfg;
  }

private:
  v2::CFG *cur_cfg = nullptr;
  // v2::CFGNode *cur_cfgnode = nullptr;
  // do not use this alone
  std::map<v2::ASTNodeBase*, v2::CFG*> Node2CFG;
  std::set<v2::CFGNode*> break_nodes;
  std::set<v2::CFGNode*> continue_nodes;
  std::set<v2::CFGNode*> return_nodes;
};

#endif /* VISITOR_H */
