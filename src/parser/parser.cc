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
  // std::cout << decls.size() << "\n";
  return new TranslationUnitDecl(Ctx, decls);
}

DeclStmt *Parser::ParseDeclStmt(XMLNode decl) {
  std::string text = get_text(decl);
  return new DeclStmt(Ctx, text);
}

FunctionDecl *Parser::ParseFunctionDecl(XMLNode node) {
  // constructnig children
  XMLNode block = node.child("block");
  CompoundStmt *comp = ParseCompoundStmt(block);
  std::string name = function_get_name(node);
  return new FunctionDecl(Ctx, name, comp);
}

CompoundStmt *Parser::ParseCompoundStmt(XMLNode node) {
  match(node, "block");
  CompoundStmt *ret = new CompoundStmt(Ctx);
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
    ret = new BreakStmt(Ctx);
  } else if (NodeName == "continue") {
    ret = new ContinueStmt(Ctx);
  } else if (NodeName == "return") {
    ret = new ReturnStmt(Ctx);
  }
  return ret;
}


IfStmt *Parser::ParseIfStmt(XMLNode node) {
  match(node, "if");
  XMLNode cond = if_get_condition_expr(node);
  Expr *condexpr = ParseExpr(cond);
  XMLNode then_node = if_get_then(node);
  Stmt *thenstmt = nullptr;
  Stmt *elsestmt = nullptr;
  if (then_node) {
    if (std::string(then_node.name()) == "block") {
      // CompoundStmt *compstmt = new CompoundStmt();
      // for (XMLNode node : then_node.children()) {
      //   compstmt->Add(ParseStmt(node));
      // }
      // thenstmt = compstmt;
      thenstmt = ParseBlockAsStmt(then_node);
    } else {
      thenstmt = ParseStmt(then_node);
    }
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
  return new IfStmt(Ctx, condexpr, thenstmt, elsestmt);
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
  SwitchStmt *ret = new SwitchStmt(Ctx, cond, NULL);
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
  CaseStmt *ret = new CaseStmt(Ctx, cond);
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
  DefaultStmt *ret = new DefaultStmt(Ctx);
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

WhileStmt *Parser::ParseWhileStmt(XMLNode node) {
  match(node, "while");
  Expr *cond = ParseExpr(while_get_condition_expr(node));
  Stmt *body = ParseBlockAsStmt(while_get_block(node));
  return new WhileStmt(Ctx, cond, body);
}

/**
 * The block can be only a single statement.
 * In this case, srcml still output <block> but with attribute
 * type="pseudo"
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
  return new ForStmt(Ctx, init, cond, inc, block);
}

DoStmt *Parser::ParseDoStmt(XMLNode node) {
  match(node, "do");
  Expr *cond = ParseExpr(node.child("condition").child("expr"));
  Stmt *block = ParseBlockAsStmt(node.child("block"));
  return new DoStmt(Ctx, cond, block);
}

Expr *Parser::ParseExpr(XMLNode node) {
  // FIXME expr might not with <expr>??
  match(node, "expr");
  return new Expr(Ctx, get_text(node));
}


void Parser::match(XMLNode node, std::string tag) {
  // std::cout << "parser cannot match: " << tag << " vs " << node.name() << "\n";
  assert(tag == node.name());
}
