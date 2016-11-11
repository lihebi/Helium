#ifndef AST_STMT_H
#define AST_STMT_H

#include "ast_node.h"


/**
 * some writeup

 Models that will add symbol table:
 1. Function
 2. Stmt
 3. For
*/

class Stmt : public ASTNode {
public:
  Stmt(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  virtual ~Stmt();
  ASTNodeKind Kind() override {return ANK_Stmt;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::set<std::string> GetVarIds() override {
    std::set<std::string> ids = get_var_ids(m_xmlnode);
    return ids;
  }
  virtual std::string GetLabel() override {
    std::string ret;
    std::set<ASTNode*> nodes;
    GetCode(nodes, ret, true);
    return ret;
  }
  virtual std::set<std::string> GetIdToResolve() override;
  virtual ASTNode* LookUpDefinition(std::string id) override;
  virtual void CreateSymbolTable() override;
private:
  std::vector<Decl*> m_decls;
};



class Block : public ASTNode {
public:
  Block(XMLNode, AST *ast, ASTNode *parent, ASTNode *prev);
  ~Block() {}
  ASTNodeKind Kind() override {return ANK_Block;}
  virtual void GetCode(std::set<ASTNode*> nodes,
                       std::string &ret, bool all) override;
  virtual std::string GetLabel() override {return "Block";}
private:
};


// function
class Function : public ASTNode {
public:
  Function(XMLNode n, AST *ast, ASTNode* parent, ASTNode *prev);
  ~Function();
  virtual ASTNodeKind Kind() override {return ANK_Function;}
  virtual std::string GetLabel() override;
  std::string GetReturnType() {return m_ret_ty;}
  std::string GetName() {return m_name;}
  void GetParams() {}
  virtual void GetCode(std::set<ASTNode*> nodes, std::string &ret, bool all) override;
  virtual std::set<std::string> GetIdToResolve() override;
  virtual void CreateSymbolTable() override;
private:
  std::string m_ret_ty;
  std::string m_name;
  std::vector<Decl*> m_params;
};



#endif /* AST_STMT_H */
