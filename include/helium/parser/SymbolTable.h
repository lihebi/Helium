#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "Visitor.h"

class SymbolTableEntry {
public:
  SymbolTableEntry(SymbolTableEntry *parent) : parent(parent) {}
  ~SymbolTableEntry() {}
  SymbolTableEntry *getParent() {return parent;}
  void add(std::string name, ASTNodeBase *node) {
    m[name] = node;
  }
  ASTNodeBase *get(std::string name) {
    if (m.count(name) == 1) return m[name];
    return nullptr;
  }
  ASTNodeBase *getRecursive(std::string name) {
    if (get(name)) return get(name);
    return parent->getRecursive(name);
  }
  std::vector<SymbolTableEntry*> getChildren() {return children;}
  void addChild(SymbolTableEntry *entry) {
    children.push_back(entry);
  }
  void dump(std::ostream &os);
private:
  SymbolTableEntry *parent = nullptr;
  std::vector<SymbolTableEntry*> children;
  std::map<std::string, ASTNodeBase*> m;
};

class SymbolTable {
public:
  SymbolTable() {
    root = new SymbolTableEntry(nullptr);
    current = root;
  }
  ~SymbolTable() {}

  void pushScope() {
    SymbolTableEntry *tbl = new SymbolTableEntry(current);
    current->addChild(tbl);
    current = tbl;
  }
  void popScope() {
    current = current->getParent();
  }
  SymbolTableEntry *getCurrent() {return current;}

  void add(std::string name, ASTNodeBase *node) {
    current->add(name, node);
  }
  ASTNodeBase* get(std::string name) {
    SymbolTableEntry *entry = current;
    while (entry) {
      ASTNodeBase *node = entry->get(name);
      if (node) return node;
      entry = entry->getParent();
    }
    return nullptr;
  }
  void bindNode(ASTNodeBase *node) {
    node2entry[node] = current;
  }
  SymbolTableEntry *getEntry(ASTNodeBase *node) {
    if (node2entry.count(node) == 1) {
      return node2entry[node];
    }
    return nullptr;
  }
  void dump(std::ostream &os);
  /**
   * get all visible ones
   */
  // std::map<std::string, ASTNodeBase*> getAll() {
  //   std::map<std::string, ASTNodeBase*> ret;
  //   for (std::map<std::string, ASTNodeBase*> v : Stack) {
  //     for (auto m : v) {
  //       ret[m.first] = m.second;
  //     }
  //   }
  //   return ret;
  // }
private:
  std::map<ASTNodeBase*, SymbolTableEntry*> node2entry;
  SymbolTableEntry *current = nullptr;
  SymbolTableEntry *root = nullptr;
};

/**
 * The symbol table should be easy to access for each node.
 * So I might consider to
 * - create the symbol table right after I create the AST
 * - store the symbol table with the AST nodes
 */
class SymbolTableBuilder : public Visitor {
public:
  SymbolTableBuilder() {
    tbl = new SymbolTable();
  }
  virtual ~SymbolTableBuilder() {}
  // high level
  virtual void visit(TokenNode *node);
  virtual void visit(TranslationUnitDecl *node);
  virtual void visit(FunctionDecl *node);
  virtual void visit(CompoundStmt *node);
  // condition
  virtual void visit(IfStmt *node);
  virtual void visit(SwitchStmt *node);
  virtual void visit(CaseStmt *node);
  virtual void visit(DefaultStmt *node);
  // loop
  virtual void visit(ForStmt *node);
  virtual void visit(WhileStmt *node);
  virtual void visit(DoStmt *node);
  // single
  virtual void visit(BreakStmt *node);
  virtual void visit(ContinueStmt *node);
  virtual void visit(ReturnStmt *node);
  // expr stmt
  virtual void visit(Expr *node);
  virtual void visit(DeclStmt *node);
  virtual void visit(ExprStmt *node);
  SymbolTable *getSymbolTable() {return tbl;}
private:
  void process(ASTNodeBase *use);
  // this will be empty after traversal
  SymbolTable *tbl = nullptr;
};


#endif /* SYMBOLTABLE_H */
