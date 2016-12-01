#ifndef AST_ITER_H
#define AST_ITER_H

#include "ast_node.h"


// loop
class For : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_For;}
  For(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~For();
  XMLNodeList GetInits() {return m_inits;}
  XMLNode GetCondition() override {return m_cond;}
  XMLNode GetIncr() {return m_incr;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override;
  virtual std::set<std::string> GetVarIds() override;
  virtual std::set<std::string> GetIdToResolve() override;
  virtual ASTNode* LookUpDefinition(std::string id) override;
  virtual void CreateSymbolTable() override;
private:
  XMLNode m_cond;
  XMLNodeList m_inits;
  XMLNode m_incr;
  std::vector<Decl*> m_decls;
};
class While : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_While;}
  While(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~While() {}
  XMLNode GetCondition() override {return m_cond;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override;
  virtual std::set<std::string> GetVarIds() override {
    return get_var_ids(m_cond);
  }
  virtual std::set<std::string> GetIdToResolve() override {
    return extract_id_to_resolve(get_text(m_cond));
  }
private:
  XMLNode m_cond;
};
class Do : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_Do;}
  Do(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~Do() {}
  XMLNode GetCondition() override {return m_cond;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override;
  virtual std::set<std::string> GetVarIds() override {
    return get_var_ids(m_cond);
  }
  virtual std::set<std::string> GetIdToResolve() override {
    return extract_id_to_resolve(get_text(m_cond));
  }
private:
  XMLNode m_cond;
};


#endif /* AST_ITER_H */
