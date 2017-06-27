#include "helium/resolver/ClangSnippet.h"
#include "helium/utils/FSUtils.h"
#include <iostream>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;


static std::vector<ClangSnippetData> Data;
static sqlite3 *db = nullptr;
std::map<std::string, std::set<std::string> > cg;


std::vector<ClangSnippetData> clangSnippetGetData() {
  return Data;
}
void clangSnippetClearData() {
  Data.clear();
  cg.clear();
}

struct Range {
  unsigned begin_l;
  unsigned begin_c;
  unsigned end_l;
  unsigned end_c;
  Range(SourceRange sr, ASTContext *ctx) {
    SourceLocation beginLoc = sr.getBegin();
    SourceLocation endLoc = sr.getEnd();
    FullSourceLoc fullBeginLoc = ctx->getFullLoc(beginLoc);
    FullSourceLoc fullEndLoc = ctx->getFullLoc(endLoc);
    begin_l = fullBeginLoc.getSpellingLineNumber();
    begin_c = fullBeginLoc.getSpellingColumnNumber();
    end_l = fullEndLoc.getSpellingLineNumber();
    end_c = fullEndLoc.getSpellingColumnNumber();
  }
};

/**
 * It should output a structure
 */
class MyVisitor
  : public RecursiveASTVisitor<MyVisitor> {
public:
  explicit MyVisitor(ASTContext *Context)
    : Context(Context) {
    SourceManager &sourceManager = Context->getSourceManager();
    const FileEntry *entry = sourceManager.getFileEntryForID(sourceManager.getMainFileID());
    Filename = entry->getName();
  }
  bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
    return true;
  }

  // function
  // enum
  // structure
  // typedef
  // union
  // var
  bool VisitFunctionDecl(FunctionDecl *func_decl) {
    std::string name = func_decl->getNameInfo().getName().getAsString();
    // errs() << "Function: " << name << "\n";
    SourceRange sourceRange = func_decl->getSourceRange();
    Range range(sourceRange, Context);
    Data.push_back({name, "function", Filename, range.begin_l, range.begin_c, range.end_l, range.end_c});
    cur_func = name;
    return true;
  }
  bool VisitVarDecl(VarDecl *var_decl) {
    // only care about file level
    if (var_decl->isFileVarDecl()) {
      std::string name = var_decl->getName();
      SourceRange sourceRange = var_decl->getSourceRange();
      Range range(sourceRange, Context);
      Data.push_back({name, "var", Filename, range.begin_l, range.begin_c, range.end_l, range.end_c});
    }
    return true;
  }
  bool VisitTypedefDecl (TypedefDecl *decl) {
    std::string name = decl->getName();
    SourceRange sourceRange = decl->getSourceRange();
    Range range(sourceRange, Context);
    Data.push_back({name, "typedef", Filename, range.begin_l, range.begin_c, range.end_l, range.end_c});
    return true;
  }
  bool VisitEnumDecl(EnumDecl *decl) {
    std::string name = decl->getName();
    SourceRange sourceRange = decl->getSourceRange();
    Range range(sourceRange, Context);
    Data.push_back({name, "enum", Filename, range.begin_l, range.begin_c, range.end_l, range.end_c});
    return true;
  }
  // struct/union/class
  bool VisitRecordDecl (RecordDecl *decl) {
    std::string name = decl->getName();
    SourceRange sourceRange = decl->getSourceRange();
    Range range(sourceRange, Context);
    Data.push_back({name, "record", Filename, range.begin_l, range.begin_c, range.end_l, range.end_c});
    return true;
  }

  bool VisitCallExpr (CallExpr *call) {
    FunctionDecl *callee = call->getDirectCallee();
    if (callee) {
      std::string callee_name = callee->getName();
      // errs() << callee_name << "\n";
      cg[cur_func].insert(callee_name);
    } else {
      errs() << "no callee! maybe in another file" << "\n";
    }
    return true;
  }

private:
  ASTContext *Context;
  std::string Filename;
  // current function, used to build call graph
  std::string cur_func;
};


class MyConsumer : public clang::ASTConsumer {
public:
  explicit MyConsumer(ASTContext *Context)
    : Visitor(Context) {}
  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  MyVisitor Visitor;
};

std::unique_ptr<clang::ASTConsumer>
MyAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
  // suppress compiler diagnostics
  Compiler.getDiagnostics().setClient(new IgnoringDiagConsumer());
  return std::unique_ptr<clang::ASTConsumer>
    (new MyConsumer(&Compiler.getASTContext()));
}




/**
 * dir should be the cache/cpp folder
 */
void clangSnippetRun(fs::path dir) {
  fs::recursive_directory_iterator it(dir), eod;
  clangSnippetClearData();
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    // std::cout << "Running clang-snippet on " << p.string() << "\n";
    if (is_regular_file(p)) {
      fs::path relative_path = fs::relative(p, dir);
      clang::tooling::runToolOnCode(
                    // new clang::SyntaxOnlyAction,
                    // clang::tooling::newFrontendActionFactory<MyAction>().get(),
                    new MyAction,
                    utils::read_file(p.string()),
                    // use relative path
                    // relative_path.string()
                    // no, use full path, so that I can get the code
                    p.string());
    }
  }
}

void clangSnippetCreateDb(fs::path db_file) {
  // fs::path db_file = (target_cache_dir/"clangSnippet.db");
  if (fs::exists(db_file)) {
    fs::remove(db_file);
  }
  // remove existing file
  // create database clangSnippet.db
  sqlite3 *db = nullptr;
  sqlite3_open(db_file.string().c_str(), &db);
  if (!db) {
    std::cerr << "Cannot open database clangSnippet.db" << "\n";
    exit(1);
  }
  // create table
  std::string cmd = "create table ClangSnippet\
      (name varchar(100), type varchar(100), file varchar(100), beginLine int, beginColumn int, endLine int, endColumn int)";
  char *errmsg = nullptr;
  int rc = sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
  }
  // create CG table
  cmd = "create table CallGraph (caller varchar(100), callee varchar(100));";
  rc =  sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, &errmsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errmsg);
    sqlite3_free(errmsg);
  }
}

void clangSnippetLoadDb(fs::path db_file) {
  // fs::path db_file = (target_cache_dir/"clangSnippet.db");
  db = nullptr;
  sqlite3_open(db_file.string().c_str(), &db);
  if (!db) {
    std::cerr << "Cannot open database clangSnippet.db" << "\n";
    exit(1);
  }
}


void clangSnippetInsertDb() {
  if (!db) {
    std::cerr << "Cannot open database clangSnippet.db" << "\n";
  }
  std::vector<ClangSnippetData> data = clangSnippetGetData();
  // std::cout << "data size: " << data.size() << "\n";
  std::string cmd = "insert into ClangSnippet values ("")";
  // insert
  std::cout << "Inserting " << data.size() << " snippets .." << "\n";
  for (ClangSnippetData d : data) {
    const char *format = "insert into ClangSnippet values ('%s','%s','%s',%d,%d,%d,%d);";
    char buf[BUFSIZ];
    snprintf(buf, BUFSIZ, format, d.name.c_str(), d.type.c_str(), d.file.c_str(),
             d.begin_line, d.begin_column, d.end_line, d.end_column);
    char *errmsg = nullptr;
    // std::cout << buf << "\n";
    int rc = sqlite3_exec(db, buf, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }
  // insert CG table
  std::cout << "Inserting Call Graph entries .." << "\n";
  int count=0;
  for (auto m : cg) {
    std::string from = m.first;
    for (std::string to : m.second) {
      count++;
      const char *format = "insert into CallGraph values ('%s', '%s');";
      char buf[BUFSIZ];
      snprintf(buf, BUFSIZ, format, from.c_str(), to.c_str());
      char *errmsg = nullptr;
      // std::cout << buf << "\n";
      int rc = sqlite3_exec(db, buf, nullptr, nullptr, &errmsg);
      if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
      }
    }
  }
  std::cout << "Inserted " << count << " call graph entries." << "\n";
}


/**
 * Query the location, aka filename, begin loc and end loc.
 */
std::string clangSnippetGetCode(std::string file, std::string kind, int line) {
  const char *format = "select beginLine,beginColumn,endLine,endColumn from ClangSnippet\
 where file='%s' and type='%s' and beginLine<=%d and endLine>=%d;";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format, file.c_str(), kind.c_str(), line, line);
  // std::string cmd = "select beginLine,beginColumn,endLine,endColumn from ClangSnippet where file="
  //   + file + " and kind=" + kind + " and beginLine<="+std::to_string(line)
  //   + " and endLine>=" + std::to_string(line) + ";";
  // std::cout << cmd << "\n";
  // cmd = "select * from ClangSnippet;";
  // std::cout << buf << "\n";
  sqlite3_stmt *stmt = nullptr;
  assert(db);
  // int rc = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);
  int rc = sqlite3_prepare_v2(db, buf, -1, &stmt, nullptr);
  assert(rc == SQLITE_OK);
  std::string ret="";
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      // data row
      int beginLine = sqlite3_column_int(stmt, 0);
      int beginColumn = sqlite3_column_int(stmt, 1);
      int endLine = sqlite3_column_int(stmt, 2);
      int endColumn = sqlite3_column_int(stmt, 3);
      // get code
      ret = utils::read_file(file, beginLine, beginColumn, endLine, endColumn);
      // std::cout << "hit: " << ret << "\n";
      // only read one record
      break;
      // return
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}


std::set<std::string> clangSnippetGetCallee(std::string caller) {
  const char *format = "select callee from CallGraph where caller='%s';";
  char buf[BUFSIZ];
  snprintf(buf, BUFSIZ, format, caller.c_str());
  sqlite3_stmt *stmt = nullptr;
  assert(db);
  // int rc = sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, nullptr);
  int rc = sqlite3_prepare_v2(db, buf, -1, &stmt, nullptr);
  assert(rc == SQLITE_OK);
  std::set<std::string> ret;
  while (true) {
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
      const unsigned char *s = sqlite3_column_text(stmt, 0);
      std::string callee((char*)s);
      ret.insert(callee);
      break;
      // return
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      assert(false);
    }
  }
  sqlite3_finalize(stmt);
  return ret;
}
