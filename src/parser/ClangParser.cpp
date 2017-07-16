#include "helium/parser/Parser.h"
#include "helium/utils/FSUtils.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Tooling/CommonOptionsParser.h"

void parseTranslationUnitDecl(clang::ASTContext *ctx,
                              clang::TranslationUnitDecl *unit) {
  clang::DeclContext::decl_iterator it;
  for (it=unit->decls_begin(); it!=unit->decls_end(); ++it) {
    clang::Decl *child = *it;
    // if (ctx->getSourceManager().isInMainFile(child->getLocStart())) {
    //   child->dump();
    // }
    // FIXME if i use a strcpy in the test program, but didn't include
    // the header file string.h, then clang will generate a top level
    // functiondecl for strcpy as implicit used function
    if (clang::FunctionDecl *func = dynamic_cast<clang::FunctionDecl*>(child)) {
      if (ctx->getSourceManager().isInMainFile(func->getLocStart())) {
        std::cout << "Got a functions." << "\n";
        std::cout << func->getName().str() << "\n";
      }
    }
  }
}

class ParserConsumer : public clang::ASTConsumer {
public:
  explicit ParserConsumer(clang::ASTContext *Context) {}
  
  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    // Then I don't need source code at all
    clang::TranslationUnitDecl *unit = Context.getTranslationUnitDecl();
    parseTranslationUnitDecl(&Context, unit);
  }
private:
};

class ParserAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    // suppress compiler diagnostics
    Compiler.getDiagnostics().setClient(new clang::IgnoringDiagConsumer());
    // return consumer
    return std::unique_ptr<clang::ASTConsumer>
      (new ParserConsumer(&Compiler.getASTContext()));
  }
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
