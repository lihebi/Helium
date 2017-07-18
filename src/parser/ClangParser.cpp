#include "helium/parser/Parser.h"
#include "helium/utils/FSUtils.h"

static SourceLocation source_loc_convert(clang::ASTContext *ctx, clang::SourceLocation loc) {
  clang::FullSourceLoc full = ctx->getFullLoc(loc);
  SourceLocation ret(full.getSpellingLineNumber(), full.getSpellingColumnNumber());
  return ret;
}

TranslationUnitDecl* ClangParser::parseTranslationUnitDecl
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::TranslationUnitDecl *unit,
 ASTContext *myctx) {
  std::vector<ASTNodeBase*> decls;
  clang::DeclContext::decl_iterator it;
  for (it=unit->decls_begin(); it!=unit->decls_end(); ++it) {
    clang::Decl *child = *it;
    if (ctx->getSourceManager().isInMainFile(child->getLocStart())) {
      // child->dump();
      // FIXME if i use a strcpy in the test program, but didn't include
      // the header file string.h, then clang will generate a top level
      // functiondecl for strcpy as implicit used function
      if (clang::FunctionDecl *func = dynamic_cast<clang::FunctionDecl*>(child)) {
        FunctionDecl *myfunc = parseFunctionDecl(ctx, rewriter, func, myctx);
        decls.push_back(myfunc);
      }
    }
  }
  clang::SourceLocation begin = unit->getLocStart();
  clang::SourceLocation end = unit->getLocEnd();
  TranslationUnitDecl *myunit = new TranslationUnitDecl(myctx, decls,
                                                        source_loc_convert(ctx, begin),
                                                        source_loc_convert(ctx, end));
  return myunit;
}

DeclStmt* ClangParser::parseDeclStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::DeclStmt *decl_stmt,
 ASTContext *myctx) {
  clang::SourceRange range = decl_stmt->getSourceRange();
  std::string mytext = rewriter.getRewrittenText(range);
  clang::SourceLocation begin = decl_stmt->getLocStart();
  clang::SourceLocation end = decl_stmt->getLocEnd();
  DeclStmt *ret = new DeclStmt(myctx, mytext,
                               source_loc_convert(ctx, begin),
                               source_loc_convert(ctx, end));
  return ret;
}
FunctionDecl *ClangParser::parseFunctionDecl
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::FunctionDecl *func,
 ASTContext *myctx) {
  clang::Stmt *stmt = func->getBody();
  // this should be a compound stmt
  assert(clang::dyn_cast<clang::CompoundStmt>(stmt));
  clang::CompoundStmt *comp = clang::dyn_cast<clang::CompoundStmt>(stmt);
  Stmt *mycomp = parseCompoundStmt(ctx, rewriter, comp, myctx);
  clang::SourceLocation begin = func->getLocStart();
  clang::SourceLocation end = func->getLocEnd();
  // FIXME nullptr
  FunctionDecl *ret = new FunctionDecl(myctx, func->getName().str(), nullptr, nullptr, nullptr, mycomp,
                                       source_loc_convert(ctx, begin),
                                       source_loc_convert(ctx, end));
  return ret;
}
CompoundStmt *ClangParser::parseCompoundStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::CompoundStmt *comp,
 ASTContext *myctx) {
  CompoundStmt *ret = new CompoundStmt(myctx, nullptr,
                                       source_loc_convert(ctx, comp->getLocStart()),
                                       source_loc_convert(ctx, comp->getLocEnd()));
  for (clang::CompoundStmt::body_iterator it=comp->body_begin(); it!=comp->body_end();++it) {
    clang::Stmt *stmt = *it;
    Stmt *mystmt = parseStmt(ctx, rewriter, stmt, myctx);
    if (mystmt) {
      ret->Add(mystmt);
    }
  }
  return ret;
}
Stmt *ClangParser::parseStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::Stmt *stmt,
 ASTContext *myctx) {
  Stmt *ret = nullptr;
  if (clang::DeclStmt *decl_stmt = clang::dyn_cast<clang::DeclStmt>(stmt)) {
    ret = parseDeclStmt(ctx, rewriter, decl_stmt, myctx);
  } else if (clang::CompoundStmt *comp = clang::dyn_cast<clang::CompoundStmt>(stmt)) {
    ret = parseCompoundStmt(ctx, rewriter, comp, myctx);
  } else if (clang::IfStmt *if_stmt = clang::dyn_cast<clang::IfStmt>(stmt)) {
    ret = parseIfStmt(ctx, rewriter, if_stmt, myctx);
  } else if (clang::SwitchStmt *switch_stmt = clang::dyn_cast<clang::SwitchStmt>(stmt)) {
    ret = parseSwitchStmt(ctx, rewriter, switch_stmt, myctx);
  } else if (clang::ForStmt *for_stmt = clang::dyn_cast<clang::ForStmt>(stmt)) {
    ret = parseForStmt(ctx, rewriter, for_stmt, myctx);
  } else if (clang::DoStmt *do_stmt = clang::dyn_cast<clang::DoStmt>(stmt)) {
    ret = parseDoStmt(ctx, rewriter, do_stmt, myctx);
  } else if (clang::WhileStmt *while_stmt = clang::dyn_cast<clang::WhileStmt>(stmt)) {
    ret = parseWhileStmt(ctx, rewriter, while_stmt, myctx);
  } else if (clang::BreakStmt *break_stmt = clang::dyn_cast<clang::BreakStmt>(stmt)) {
    ret = parseBreakStmt(ctx, rewriter, break_stmt, myctx);
  } else if (clang::ContinueStmt *cont_stmt = clang::dyn_cast<clang::ContinueStmt>(stmt)) {
    ret = parseContinueStmt(ctx, rewriter, cont_stmt, myctx);
  } else if (clang::ReturnStmt *ret_stmt = clang::dyn_cast<clang::ReturnStmt>(stmt)) {
    ret = parseReturnStmt(ctx, rewriter, ret_stmt, myctx);
  }
  return ret;
};
ReturnStmt *ClangParser::parseReturnStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::ReturnStmt *ret_stmt,
 ASTContext *myctx) {
  clang::Expr *expr = ret_stmt->getRetValue();
  // std::string text = rewriter.getRewrittenText(expr->getSourceRange());
  Expr *myexpr = parseExpr(ctx, rewriter, expr, myctx);
  ReturnStmt *ret = new ReturnStmt(myctx, nullptr, myexpr,
                                   source_loc_convert(ctx, ret_stmt->getLocStart()),
                                   source_loc_convert(ctx, ret_stmt->getLocEnd()));
  return ret;
}
IfStmt *ClangParser::parseIfStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::IfStmt *if_stmt,
 ASTContext *myctx) {
  clang::Expr *cond = if_stmt->getCond();
  Expr *mycond = parseExpr(ctx, rewriter, cond, myctx);
  clang::Stmt *then_stmt = if_stmt->getThen();
  clang::Stmt *else_stmt = if_stmt->getElse();
  Stmt *mythen = parseStmt(ctx, rewriter, then_stmt, myctx);
  Stmt *myelse = parseStmt(ctx, rewriter, else_stmt, myctx);

  IfStmt *ret = new IfStmt(myctx, mycond, mythen, myelse, nullptr, nullptr,
                           source_loc_convert(ctx, if_stmt->getLocStart()),
                           source_loc_convert(ctx, if_stmt->getLocEnd()));
  return ret;
}
SwitchStmt *ClangParser::parseSwitchStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::SwitchStmt *switch_stmt,
 ASTContext *myctx) {
  clang::Expr *cond = switch_stmt->getCond();
  Expr *mycond = parseExpr(ctx, rewriter, cond, myctx);
  SwitchStmt *ret = new SwitchStmt(myctx, mycond, nullptr, nullptr,
                                   source_loc_convert(ctx, switch_stmt->getLocStart()),
                                   source_loc_convert(ctx, switch_stmt->getLocEnd()));
  // case
  clang::SwitchCase *switch_case =  switch_stmt->getSwitchCaseList();
  while (switch_case) {
    if (clang::CaseStmt *case_stmt = clang::dyn_cast<clang::CaseStmt>(switch_case)) {
      CaseStmt *mycase = parseCaseStmt(ctx, rewriter, case_stmt, myctx);
      ret->AddCase(mycase);
    } else if (clang::DefaultStmt *def_stmt = clang::dyn_cast<clang::DefaultStmt>(switch_case)) {
      DefaultStmt *mydef = parseDefaultStmt(ctx, rewriter, def_stmt, myctx);
      ret->AddCase(mydef);
    }
    switch_case = switch_case->getNextSwitchCase();
  }
  return ret;
}
CaseStmt *ClangParser::parseCaseStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::CaseStmt *case_stmt,
 ASTContext *myctx) {
  // FIXME cond?? rhs?
  clang::Expr *expr = case_stmt->getRHS();
  Expr *myexpr = parseExpr(ctx, rewriter, expr, myctx);

  CaseStmt *ret = new CaseStmt(myctx, myexpr, nullptr,
                               source_loc_convert(ctx, case_stmt->getLocStart()),
                               source_loc_convert(ctx, case_stmt->getLocEnd()));
  // FIXME
  // get substatement
  clang::Stmt *sub = case_stmt->getSubStmt();
  Stmt *mysub = parseStmt(ctx, rewriter, sub, myctx);
  ret->Add(mysub);
  return ret;
}
DefaultStmt *ClangParser::parseDefaultStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::DefaultStmt *def_stmt,
 ASTContext *myctx) {

  DefaultStmt *ret = new DefaultStmt(myctx, nullptr,
                                     source_loc_convert(ctx, def_stmt->getLocStart()),
                                     source_loc_convert(ctx, def_stmt->getLocEnd()));
  clang::Stmt *sub = def_stmt->getSubStmt();
  Stmt *mysub = parseStmt(ctx, rewriter, sub, myctx);
  ret->Add(mysub);
  return ret;
}
WhileStmt *ClangParser::parseWhileStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::WhileStmt *while_stmt,
 ASTContext *myctx) {
  clang::Expr *cond = while_stmt->getCond();
  Expr *mycond = parseExpr(ctx, rewriter, cond, myctx);
  clang::Stmt *body = while_stmt->getBody();
  Stmt *mybody = parseStmt(ctx, rewriter, body, myctx);
  WhileStmt *ret = new WhileStmt(myctx, mycond, mybody, nullptr,
                                 source_loc_convert(ctx, while_stmt->getLocStart()),
                                 source_loc_convert(ctx, while_stmt->getLocEnd()));
  return ret;
}
ForStmt *ClangParser::parseForStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::ForStmt *for_stmt,
 ASTContext *myctx) {
  clang::Stmt *init = for_stmt->getInit();
  clang::Expr *cond = for_stmt->getCond();
  clang::Expr *inc = for_stmt->getInc();
  clang::Stmt *body = for_stmt->getBody();
  // FIXME expr or stmt?
  // Stmt *myinit = parseStmt(ctx, rewriter, init, myctx);
  Expr *myinit = nullptr;
  Expr *mycond = parseExpr(ctx, rewriter, cond, myctx);
  Expr *myinc = parseExpr(ctx, rewriter, inc, myctx);
  Stmt *mybody = parseStmt(ctx, rewriter, body, myctx);
  ForStmt *ret = new ForStmt(myctx, myinit, mycond, myinc, mybody, nullptr,
                             source_loc_convert(ctx, for_stmt->getLocStart()),
                             source_loc_convert(ctx, for_stmt->getLocEnd()));
  return ret;
}
DoStmt *ClangParser::parseDoStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::DoStmt *do_stmt,
 ASTContext *myctx) {
  clang::Expr *cond = do_stmt->getCond();
  clang::Stmt *body = do_stmt->getBody();
  Expr *mycond = parseExpr(ctx, rewriter, cond, myctx);
  Stmt *mybody = parseStmt(ctx, rewriter, body, myctx);
  DoStmt *ret = new DoStmt(myctx, mycond, mybody, nullptr, nullptr,
                           source_loc_convert(ctx, do_stmt->getLocStart()),
                           source_loc_convert(ctx, do_stmt->getLocEnd()));
  return ret;
}
Expr *ClangParser::parseExpr
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::Expr *expr,
 ASTContext *myctx) {
  clang::SourceRange range = expr->getSourceRange();
  std::string text = rewriter.getRewrittenText(range);
  Expr *ret = new Expr(myctx, text,
                       source_loc_convert(ctx, expr->getLocStart()),
                       source_loc_convert(ctx, expr->getLocEnd()));
  return ret;
}
BreakStmt *ClangParser::parseBreakStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::BreakStmt *break_stmt,
 ASTContext *myctx) {
  BreakStmt *ret = new BreakStmt(myctx,
                                 source_loc_convert(ctx, break_stmt->getLocStart()),
                                 source_loc_convert(ctx, break_stmt->getLocEnd()));
  return ret;
}
ContinueStmt *ClangParser::parseContinueStmt
(clang::ASTContext *ctx, clang::Rewriter &rewriter,
 clang::ContinueStmt *cont_stmt,
 ASTContext *myctx) {
  ContinueStmt *ret = new ContinueStmt(myctx,
                                       source_loc_convert(ctx, cont_stmt->getLocStart()),
                                       source_loc_convert(ctx, cont_stmt->getLocEnd()));
  return ret;
}

class ParserConsumer : public clang::ASTConsumer {
public:
  explicit ParserConsumer(clang::ASTContext *Context, clang::Rewriter &rewriter)
    : rewriter(rewriter) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    // Then I don't need source code at all
    clang::TranslationUnitDecl *unit = Context.getTranslationUnitDecl();
    ASTContext *myctx = new ASTContext("dummy-filename.c");
    ClangParser::parseTranslationUnitDecl(&Context, rewriter, unit, myctx);
  }
private:
  clang::Rewriter &rewriter;
};

class ParserAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    // suppress compiler diagnostics
    Compiler.getDiagnostics().setClient(new clang::IgnoringDiagConsumer());
    rewriter.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
    // return consumer
    return std::unique_ptr<clang::ASTConsumer>
      (new ParserConsumer(&Compiler.getASTContext(), rewriter));
  }
private:
  clang::Rewriter rewriter;
};

void create_by_action(fs::path file) {
  // read from file
  std::string code = utils::read_file(file);
  ParserAction *action = new ParserAction();
  clang::tooling::runToolOnCode(action, code, file.string());
  // Now I should be able to get data from action
}

// Unused
void create_by_build(fs::path file) {
  std::string code = utils::read_file(file);
  // std::unique_ptr<clang::ASTUnit> ast =
  //   clang::tooling::buildASTFromCode(code, file.string());
  std::vector<std::string> args = {"-I/usr/include/linux"};
  std::unique_ptr<clang::ASTUnit> ast =
    clang::tooling::buildASTFromCodeWithArgs(code, args, file.string());
  // clang::ASTContext *ctx = &(ast->getASTContext());
  // setting ignore here doesn't work because the error is already thrown
  // ctx->getDiagnostics().setClient(new clang::IgnoringDiagConsumer());
  // clang::TranslationUnitDecl *unit = ctx->getTranslationUnitDecl();
  // parseTranslationUnitDecl(unit);
}

// void create_by_tool(fs::path file) {
//   ToolInvocation invo()
// }


ASTContext *ClangParser::parse(fs::path file) {
  ASTContext *ctx = new ASTContext(file.string());
  create_by_action(file);


  TranslationUnitDecl *unit = nullptr;
  // TODO
  // unit = ParseTranslationUnitDecl(root);
  ctx->setTranslationUnitDecl(unit);
  return ctx;
}

#if 0
void create_by_test(fs::path file) {
  CompilerInstance compiler;
  DiagnosticOptions options;
  compiler.createDisagnostics();
  CompilerInvocation *invocation = new CompilerInvocation;
  CompilerInvocation::CreateFromArgs(*invocation, NULL, NULL, compiler.getDiagnostics());
  compiler.setInvocation(invocation);
  std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
  pto->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *pti = TargetInfo::CreateTargetInfo(compiler.getDiagnostics(), pto);
  compiler.setTarget(pti);
  compiler.createFileManager();
  compiler.createSourceManager(compiler.getFileManager());
  HeaderSearchOptions &headerSearchOptions = compiler.getHeaderSearchOpts();

  LangOptions langOpts;
  langOpts.GNUMode = 1;
  invocation->setLangDefaults(langOpts);
  compiler.createPreprocessor(clang::TU_Complete);
  compiler.getPreprocessorOpts().UsePredefines = false;
  compiler.createASTContext();
}
#endif
