#include "helium/parser/parser.h"
#include "helium/parser/xml_doc_reader.h"
#include "helium/parser/ast_v2.h"

#include "helium/parser/xmlnode_helper.h"


using std::vector;
using namespace v2;

// rename this file to srcmlparser.cc

Parser::Parser(std::string filename) {
    XMLDoc *doc = XMLDocReader::CreateDocFromFile(filename);
    XMLNode root = doc->document_element();
    Ctx = new ASTContext(filename);
    Ctx->setTranslationUnitDecl(ParseTranslationUnitDecl(root));
}

TranslationUnitDecl *Parser::ParseTranslationUnitDecl(XMLNode unit) {
  match(unit, "unit");
  // std::vector<DeclStmt*> decls;
  // std::vector<FunctionDecl*> funcs;
  vector<ASTNodeBase*> decls;
  for (XMLNode node : unit.children()) {
    if (std::string(node.name()) == "decl_stmt") {
      DeclStmt *decl = ParseDeclStmt(node);
      decls.push_back(decl);
    } else if (std::string(node.name()) == "function") {
      FunctionDecl *func = ParseFunctionDecl(node);
      // funcs.push_back(func);
      decls.push_back(func);
    }
  }

  // get the source location of unit
  std::pair<int, int> begin = get_node_begin_position(unit);
  std::pair<int, int> end = get_node_end_position(unit);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  // std::cout << decls.size() << "\n";
  return new TranslationUnitDecl(Ctx, decls, BeginLoc, EndLoc);
}

DeclStmt *Parser::ParseDeclStmt(XMLNode decl) {
  std::string text = get_text(decl);
  std::pair<int, int> begin = get_node_begin_position(decl);
  std::pair<int, int> end = get_node_end_position(decl);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  return new DeclStmt(Ctx, text, BeginLoc, EndLoc);
}

/**
 * Make a TokenNode out of an XMLNode
 */
TokenNode *make_token_node(ASTContext *ctx, XMLNode node, std::string text) {
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_begin_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  return new TokenNode(ctx, text, BeginLoc, EndLoc);
}

FunctionDecl *Parser::ParseFunctionDecl(XMLNode node) {
  // constructnig children
  XMLNode block = node.child("block");
  CompoundStmt *comp = ParseCompoundStmt(block);
  std::string name = function_get_name(node);
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);

  // TokenNodes
  TokenNode *ReturnTypeNode = make_token_node(Ctx, node.child("type"), get_text(node.child("type")));
  TokenNode *NameNode = make_token_node(Ctx, node.child("name"), get_text(node.child("name")));
  // caution: this contains parenthesis
  TokenNode *ParamNode = make_token_node(Ctx, node.child("parameter_list"),
                                         get_text(node.child("parameter_list")));

  return new FunctionDecl(Ctx, name, ReturnTypeNode, NameNode, ParamNode, comp, BeginLoc, EndLoc);
}

CompoundStmt *Parser::ParseCompoundStmt(XMLNode node) {
  match(node, "block");
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  CompoundStmt *ret = new CompoundStmt(Ctx, BeginLoc, EndLoc);
  for (XMLNode n : element_children(node)) {
    std::string NodeName = n.name();
    Stmt *stmt = ParseStmt(n);
    ret->Add(stmt);
  }
  return ret;
}

/**
 * Parse all kinds of statements here
 */
Stmt *Parser::ParseStmt(XMLNode node) {
  std::string NodeName = node.name();
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  Stmt *ret = nullptr;
  if (NodeName == "decl_stmt") {
    ret = ParseDeclStmt(node);
  } else if (NodeName == "stmt") {
    // FIXME srcml does not only contain these two
    // e.g. it might directly be a if statement
    // Stmt *stmt = ParseStmt(node);
    // FIXME no such tag in srmcl? will cause recursion
    // error();
    assert(0);
  } else if (NodeName == "if") {
    ret = ParseIfStmt(node);
  } else if (NodeName == "while") {
    ret = ParseWhileStmt(node);
  } else if (NodeName == "break") {
    ret = new BreakStmt(Ctx, BeginLoc, EndLoc);
  } else if (NodeName == "continue") {
    ret = new ContinueStmt(Ctx, BeginLoc, EndLoc);
  } else if (NodeName == "return") {
    SourceLocation TokenBeginLoc(begin.first, begin.second);
    SourceLocation TokenEndLoc(begin.first, begin.second + strlen("return"));
    TokenNode *ReturnNode = new TokenNode(Ctx, "return", TokenBeginLoc, TokenEndLoc);
    ret = new ReturnStmt(Ctx, ReturnNode, BeginLoc, EndLoc);
  }
  return ret;
}


IfStmt *Parser::ParseIfStmt(XMLNode node) {
  match(node, "if");
  XMLNode cond = if_get_condition_expr(node);
  Expr *condexpr = ParseExpr(cond);
  XMLNode then_node = if_get_then(node);
  assert(then_node);
  Stmt *thenstmt = nullptr;
  Stmt *elsestmt = nullptr;
  if (then_node) {
    thenstmt = ParseBlockAsStmt(then_node.child("block"));
    // if (std::string(then_node.name()) == "block") {
    //   thenstmt = ParseBlockAsStmt(then_node);
    // } else {
    //   thenstmt = ParseStmt(then_node);
    // }
  } else {
    std::cerr << "Error: no else for if." << "\n";
    exit(1);
  }
  XMLNode elseifnode = node.child("elseif");
  if (elseifnode) {
    elsestmt = ParseElseIfAsIfStmt(elseifnode);
  }
  XMLNode elsenode = node.child("else");
  if (elsenode) {
    elsestmt = ParseBlockAsStmt(elsenode.child("block"));
  }
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);

  // TokenNode
  std::pair<int, int> if_begin = get_node_begin_position(node);
  SourceLocation IfNodeBegin(if_begin.first, if_begin.second);
  SourceLocation IfNodeEnd(if_begin.first, if_begin.second + strlen("if"));
  TokenNode *IfNode = new TokenNode(Ctx, "if", IfNodeBegin, IfNodeEnd);
  
  TokenNode *ElseNode = nullptr;
  if (elsestmt) {
    // this might be <else> or <elseif>
    std::pair<int,int> else_begin;
    if (elsenode) else_begin = get_node_begin_position(elsenode);
    else if (elseifnode) else_begin = get_node_begin_position(elseifnode);
    SourceLocation ElseNodeBegin(else_begin.first, else_begin.second);
    SourceLocation ElseNodeEnd(else_begin.first, else_begin.second + strlen("else"));
    ElseNode = new TokenNode(Ctx, "else", ElseNodeBegin, ElseNodeEnd);
  }
  return new IfStmt(Ctx, condexpr, thenstmt, elsestmt, IfNode, ElseNode, BeginLoc, EndLoc);
}

/**
 * <if><then></then>
 *     <elseif><if></if></elseif>
 *     <elseif></elseif>
 *     <else></else>
 * </if>
 *
 * Into
 *
 * (if (then)
 *     (if (then)
 *         (if (then)
 *         (else))))
 */
IfStmt *Parser::ParseElseIfAsIfStmt(XMLNode node) {
  match(node, "elseif");
  IfStmt *ifstmt = ParseIfStmt(node.child("if"));
  // XMLNode next = node.next_sibling();
  XMLNode next = next_element_sibling(node);
  Stmt *elsestmt = nullptr;
  if (next) {
    if (std::string(next.name()) == "elseif") {
      elsestmt = ParseElseIfAsIfStmt(next);
    } else if (std::string(next.name()) == "else") {
      elsestmt = ParseBlockAsStmt(next.child("block"));
    } else {
      // error();
      // std::cout << next.name() << "\n";
      assert(0);
    }
  }
  ifstmt->setElse(elsestmt);
  return ifstmt;
}


SwitchStmt *Parser::ParseSwitchStmt(XMLNode node) {
  match(node, "switch");
  Expr *cond = ParseExpr(node.child("condition").child("expr"));
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);

  // SwitchNode
  SourceLocation SwitchNodeBegin(begin.first, begin.second);
  SourceLocation SwitchNodeEnd(begin.first, begin.second + strlen("switch"));
  TokenNode *SwitchNode = new TokenNode(Ctx, "switch", SwitchNodeBegin, SwitchNodeEnd);
  
  SwitchStmt *ret = new SwitchStmt(Ctx, cond, NULL, SwitchNode, BeginLoc, EndLoc);
  for(XMLNode c : node.child("block").children("case")) {
    CaseStmt *casestmt = ParseCaseStmt(c);
    ret->AddCase(casestmt);
  }
  XMLNode defnode = node.child("block").child("default");
  if (defnode) {
    ret->AddCase(ParseDefaultStmt(defnode));
  }
  return ret;
}

/**
 * <case><expr></expr></caes>
 * <expr_stmt>
 * <break>
 * <case>
 * <default>
 */
CaseStmt *Parser::ParseCaseStmt(XMLNode node) {
  Expr *cond = ParseExpr(node.child("expr"));
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  // CaseNode
  SourceLocation CaseNodeBegin(begin.first, begin.second);
  SourceLocation CaseNodeEnd(begin.first, begin.second = strlen("case"));
  TokenNode *CaseNode = new TokenNode(Ctx, "case", CaseNodeBegin, CaseNodeEnd);
  CaseStmt *ret = new CaseStmt(Ctx, cond, CaseNode, BeginLoc, EndLoc);
  XMLNode n = node;
  while ((n = next_element_sibling(n))) {
    if (std::string(n.name()) == "case" ||
        std::string(n.name()) == "default") {
      break;
    }
    ret->Add(ParseStmt(n));
  }
  return ret;
}
DefaultStmt *Parser::ParseDefaultStmt(XMLNode node) {
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  // DefaultNode
  SourceLocation DefaultNodeBegin(begin.first, begin.second);
  SourceLocation DefaultNodeEnd(begin.first, begin.second + strlen("default"));
  TokenNode *DefaultNode = new TokenNode(Ctx, "default", DefaultNodeBegin, DefaultNodeEnd);
  
  DefaultStmt *ret = new DefaultStmt(Ctx, DefaultNode, BeginLoc, EndLoc);
  XMLNode n = node;
  while ((n = next_element_sibling(n))) {
    if (std::string(n.name()) == "case" ||
        std::string(n.name()) == "default") {
      break;
    }
    ret->Add(ParseStmt(n));
  }
  return ret;
}

/**
 * The block can be only a single statement.
 * In this case, srcml still output <block> but with attribute
 * type="pseudo"
 * FIXME should I keep the block even if it is pseudo?
 */
Stmt *Parser::ParseBlockAsStmt(XMLNode node) {
  match(node, "block");
  // XMLNode block = node.child("block");
  Stmt *ret = nullptr;
  if (std::string(node.attribute("type").value()) == "pseudo") {
    XMLNode child = node.first_child();
    ret = ParseStmt(child);
  } else {
    ret = ParseCompoundStmt(node);
  }
  return ret;
}

ForStmt *Parser::ParseForStmt(XMLNode node) {
  match(node, "for");
  Expr *init = ParseExpr(node.child("control").child("init"));
  Expr *cond = ParseExpr(node.child("control").child("condition"));
  Expr *inc = ParseExpr(node.child("control").child("incr"));
  Stmt *block = ParseBlockAsStmt(node.child("block"));
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);

  // make up for for token
  SourceLocation TokenBeginLoc(begin.first, begin.second);
  SourceLocation TokenEndLoc(begin.first, begin.second + strlen("for"));
  TokenNode *ForNode = new TokenNode(Ctx, "for", TokenBeginLoc, TokenEndLoc);
  
  return new ForStmt(Ctx, init, cond, inc, block, ForNode, BeginLoc, EndLoc);
}

WhileStmt *Parser::ParseWhileStmt(XMLNode node) {
  match(node, "while");
  Expr *cond = ParseExpr(while_get_condition_expr(node));
  Stmt *body = ParseBlockAsStmt(while_get_block(node));
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  // WhileNode
  SourceLocation TokenBeginLoc(begin.first, begin.second);
  SourceLocation TokenEndLoc(begin.first, begin.second + strlen("while"));
  TokenNode *WhileNode = new TokenNode(Ctx, "while", TokenBeginLoc, TokenEndLoc);
  return new WhileStmt(Ctx, cond, body, WhileNode, BeginLoc, EndLoc);
}


DoStmt *Parser::ParseDoStmt(XMLNode node) {
  match(node, "do");
  Expr *cond = ParseExpr(node.child("condition").child("expr"));
  Stmt *block = ParseBlockAsStmt(node.child("block"));
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);

  // DoNode
  SourceLocation TokenBeginLoc(begin.first, begin.second);
  SourceLocation TokenEndLoc(begin.first, begin.second + strlen("do"));
  TokenNode *DoNode = new TokenNode(Ctx, "do", TokenBeginLoc, TokenEndLoc);

  // WhileNode: this is not precise
  XMLNode blockNode = do_get_block(node);
  XMLNode condNode = do_get_condition(node);
  std::pair<int, int> block_end = get_node_end_position(blockNode);
  SourceLocation WhileBeginLoc(block_end.first, block_end.second);
  std::pair<int, int> cond_begin = get_node_begin_position(condNode);
  SourceLocation WhileEndLoc(cond_begin.first, cond_begin.second);
  TokenNode *WhileNode = new TokenNode(Ctx, "while", WhileBeginLoc, WhileEndLoc);
  
  return new DoStmt(Ctx, cond, block, DoNode, WhileNode, BeginLoc, EndLoc);
}

Expr *Parser::ParseExpr(XMLNode node) {
  // FIXME expr might not with <expr>??
  match(node, "expr");
  std::pair<int, int> begin = get_node_begin_position(node);
  std::pair<int, int> end = get_node_end_position(node);
  SourceLocation BeginLoc(begin.first, begin.second);
  SourceLocation EndLoc(end.first, end.second);
  return new Expr(Ctx, get_text(node), BeginLoc, EndLoc);
}


void Parser::match(XMLNode node, std::string tag) {
  // std::cout << "parser cannot match: " << tag << " vs " << node.name() << "\n";
  assert(tag == node.name());
}
