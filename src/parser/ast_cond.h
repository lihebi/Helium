#ifndef AST_COND_H
#define AST_COND_H

#include "ast_node.h"

/*******************************
 * If
 *******************************/

class Then;
class Else;
class ElseIf;
class Case;
class Default;

class If : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_If;}
  If(XMLNode xmlnode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~If() {}
  XMLNode GetCondition() override {return m_cond;}
  Then* GetThen() {return m_then;}
  std::vector<ElseIf*> GetElseIfs() {return m_elseifs;}
  Else* GetElse() {return m_else;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override;
  virtual std::set<std::string> GetVarIds() override {
    return get_var_ids(m_cond);
  }
  virtual std::set<std::string> GetIdToResolve() override {
    return extract_id_to_resolve(get_text(m_cond));
  }
  // virtual ASTNode* LookUpDefinition(std::string id) override;

private:
  XMLNode m_cond;
  Then *m_then = NULL;
  Else *m_else = NULL;
  std::vector<ElseIf*> m_elseifs;
};


class Else : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_Else;}
  Else(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~Else() {}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override {return "Else";}
private:
};
  
class ElseIf : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_ElseIf;}
  ElseIf(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
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
  // virtual ASTNode* LookUpDefinition(std::string id) override;
private:
  XMLNode m_cond;
};
  
class Then : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_Then;}
  Then(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override {return "Then";}
private:
};


/*******************************
 * Switch
 *******************************/

/*******************************
 * Swith
 *******************************/
class Switch : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_Switch;}
  Switch(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~Switch() {}
  XMLNode GetCondition() override {return m_cond;}
  std::vector<Case*> GetCases() {return m_cases;}
  Default* GetDefault() {return m_default;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override;
  virtual std::set<std::string> GetVarIds() override {
    return get_var_ids(m_cond);
  }
private:
  XMLNode m_cond;
  std::vector<Case*> m_cases;
  Default *m_default = NULL;
};

class Case : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_Case;}
  Case(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override;
private:
  XMLNode m_cond;
};
class Default : public ASTNode {
public:
  ASTNodeKind Kind() override {return ANK_Default;}
  Default(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override {return "Default";}
private:
};




#endif /* AST_COND_H */
