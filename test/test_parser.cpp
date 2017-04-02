#include "helium/parser/parser.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "helium/parser/GrammarPatcher.h"
#include "helium/utils/string_utils.h"

#include "test_programs.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;
using namespace v2;

using namespace std;

class VisitorTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    fs::path dir = fs::temp_directory_path();
    fs::create_directories(dir);

    for (int i=0;i<programs.size();i++) {
      fs::path source = dir / (std::to_string(i) + ".c");
      std::ofstream of (source.string());
      ASSERT_TRUE(of.is_open());
      std::string prog = programs[i];
      // remove white spaces so that i can count the location
      utils::trim(prog);
      of << prog;
      of.close();
      Parser *parser = new Parser(source.string());
      parsers.push_back(parser);
      asts.push_back(parser->getASTContext());
    }
  }

  virtual void TearDown() {}
  vector<Parser*> parsers;
  vector<ASTContext*> asts;
};

void dummy()  {
  GrammarPatcher patcher;
  Matcher matcher;
}

// TEST_F(VisitorTest, ParserTest) {
//   {
//     ASTContext *ast = asts[0];
//     TranslationUnitDecl *decl = ast->getTranslationUnitDecl();
//     vector<ASTNodeBase*> decls = decl->getDecls();
//     ASSERT_EQ(decls.size(), 1);
//   }
// }

TEST_F(VisitorTest, LevelVisitorTest) {
  {
    LevelVisitor visitor;
    Matcher matcher;
    TranslationUnitDecl *unit = asts[0]->getTranslationUnitDecl();
    unit->accept(&visitor);
    unit->accept(&matcher);
    EXPECT_EQ(visitor.getLevel(unit), 0);

    // matcher.dump(std::cout);

    vector<ASTNodeBase*> v;
    v = matcher.match("TranslationUnitDecl/FunctionDecl/CompoundStmt/IfStmt");
    // std::cout << matcher.size() << "\n";
    // matcher.dump(std::cout);
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(visitor.getLevel(v[0]), 3);
    v = matcher.match("TranslationUnitDecl/FunctionDecl/CompoundStmt/IfStmt/Expr");
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(visitor.getLevel(v[0]), 4);
  }
}


// TEST_F(VisitorTest, PrinterTest) {
//   for (ASTContext *ast : asts) {
//     Printer printer;
//     TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
//     unit->accept(&printer);
//     std::string s = printer.getString();
//     // std::cout << s << "\n";
//   }
// }

// TEST_F(VisitorTest, TokenVisitorTest) {
//   {
//     TokenVisitor visitor;
//     TranslationUnitDecl *unit = asts[0]->getTranslationUnitDecl();
//     unit->accept(&visitor);
//     vector<ASTNodeBase*> tokens = visitor.getTokens();
//     ASSERT_EQ(tokens.size(), 6);
//   }
// }

TEST_F(VisitorTest, ParentIndexerTest) {
  {
    ParentIndexer indexer;
    Matcher matcher;
    TranslationUnitDecl *unit = asts[0]->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);
    vector<ASTNodeBase*> v;
    v = matcher.match("TranslationUnitDecl/FunctionDecl/CompoundStmt/IfStmt");
    // std::cout << matcher.size() << "\n";
    // matcher.dump(std::cout);
    ASSERT_EQ(v.size(), 1);
    ASTNodeBase *comp = indexer.getParent(v[0]);
    ASSERT_TRUE(comp);
    EXPECT_EQ(comp->getNodeName(), "CompoundStmt");
    ASTNodeBase *func = indexer.getParent(comp);
    ASSERT_TRUE(func);
    EXPECT_EQ(func->getNodeName(), "FunctionDecl");
    vector<ASTNodeBase*> children = indexer.getChildren(func);
    EXPECT_EQ(children.size(), 4); // int, foo, (), comp
    EXPECT_NE(std::find(children.begin(), children.end(), comp), children.end());
    children = indexer.getChildren(comp);
    EXPECT_EQ(children.size(), 2);
    children = indexer.getChildren(v[0]);
    // for (auto *c : children) {
    //   c->dump(std::cout);
    // }
    // std::cout  << "\n";
    ASSERT_EQ(children.size(), 3);
    EXPECT_EQ(children[0]->getNodeName(), "TokenNode");
    EXPECT_EQ(children[1]->getNodeName(), "Expr");
    EXPECT_EQ(children[2]->getNodeName(), "CompoundStmt");
    // EXPECT_EQ(children[3]->getNodeName(), "TokenNode");
    // EXPECT_EQ(children[4]->getNodeName(), "CompStmt");
  }
}

TEST_F(VisitorTest, GrammarPatcherTest) {
  // Program 1
  // -------------------
  // int foo() {
  //   if (b>0) {
  //     d=c+e;
  //   }
  // }
  // -------------------
  // - ifnode
  // - IF, cond, comp
  // - {}, expr_stmt
  {

    // set up indexer and matcher for selecting nodes
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[0];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);

    // DEBUG
    // Printer printer;
    // unit->accept(&printer);
    // std::cout << printer.getString() << "\n";

    
    // level 1
    ASTNodeBase *if_stmt = matcher.getNodeByLoc("IfStmt", 2);
    // level 2
    ASTNodeBase *if_token = matcher.getNodeByLoc("TokenNode", 2);
    ASTNodeBase *if_cond = matcher.getNodeByLoc("Expr", 2);
    ASTNodeBase *comp = matcher.getNodeByLoc("CompoundStmt", 2);
    // level 3
    ASTNodeBase *comp_token = matcher.getNodeByLoc("TokenNode", 2, 1);

    ASSERT_TRUE(if_stmt);
    ASSERT_TRUE(if_token);
    ASSERT_TRUE(if_cond);
    ASSERT_TRUE(comp);
    ASSERT_TRUE(comp_token);
    // if_stmt->dump(std::cout);
    ASSERT_EQ(if_stmt->getNodeName(), "IfStmt");
    ASSERT_EQ(if_token->getNodeName(), "TokenNode");
    ASSERT_EQ(if_cond->getNodeName(), "Expr");
    ASSERT_EQ(comp->getNodeName(), "CompoundStmt");
    ASSERT_EQ(comp_token->getNodeName(), "TokenNode");
    {
      /**
       * (HEBI: Test selection of if header token)
       */
      // create selection
      // if token
      // output:
      // if (b>0) {}
      // - ifnode
      // - IF, cond, comp
      // - {}
      set<ASTNodeBase*> sel;
      sel.insert(if_token);

      // create GrammarPatcher
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();

      EXPECT_EQ(patch.size(), 5);

      // test the 4 components stored in patch
      EXPECT_EQ(patch.count(if_stmt), 1);
      EXPECT_EQ(patch.count(if_token), 1);
      EXPECT_EQ(patch.count(if_cond), 1);
      EXPECT_EQ(patch.count(comp), 1);
      EXPECT_EQ(patch.count(comp_token), 1);
    }
  }
  // Program 2
// int foo() {
//   if (b>0) {
//     d=c+e;
//   } else if (b < 10) {
//     a=8;
//   }
// }
  {
    // set up indexer and matcher for selecting nodes
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[1];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);

    // DEBUG
    // Printer printer;
    // unit->accept(&printer);
    // std::cout << printer.getString() << "\n";

    // level 1
    ASTNodeBase *if_stmt = matcher.getNodeByLoc("IfStmt", 2);
    // level 2
    ASTNodeBase *if_token = matcher.getNodeByLoc("TokenNode", 2);
    ASTNodeBase *if_cond = matcher.getNodeByLoc("Expr", 2);
    ASTNodeBase *comp = matcher.getNodeByLoc("CompoundStmt", 2);
    ASTNodeBase *else_token = matcher.getNodeByLoc("TokenNode", 4);
    ASTNodeBase *if_stmt_2 = matcher.getNodeByLoc("IfStmt", 4);
    // level 3
    ASTNodeBase *comp_token = matcher.getNodeByLoc("TokenNode", 2, 1);
    ASTNodeBase *if_token_2 = matcher.getNodeByLoc("TokenNode", 4, 1);
    ASTNodeBase *if_cond_2 = matcher.getNodeByLoc("Expr", 4);
    ASTNodeBase *comp_2 = matcher.getNodeByLoc("CompoundStmt", 4);
    // level 4
    ASTNodeBase *comp_token_2 = matcher.getNodeByLoc("TokenNode", 4, 2);
    ASSERT_TRUE(comp_token_2);
    /**
     * (HEBI: Test selection of else in if)
     */
    {
      /**
       * Select:
       * IF, ELSE
       * output:
       * if (b>0) {} else if (b<10) {}
       * - if
       * - IF, COND, comp, ELSE, if
       * - {}, IF, COND, comp
       * - {}
       */
      set<ASTNodeBase*> sel;
      sel.insert(if_token);
      sel.insert(else_token);

      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      
      ASSERT_EQ(patch.size(), 11);

      // level 1
      EXPECT_EQ(patch.count(if_stmt), 1);
      // level 2
      EXPECT_EQ(patch.count(if_token), 1);
      EXPECT_EQ(patch.count(if_cond), 1);
      EXPECT_EQ(patch.count(comp), 1);
      EXPECT_EQ(patch.count(else_token), 1);
      EXPECT_EQ(patch.count(if_stmt_2), 1);
      // level 3
      EXPECT_EQ(patch.count(comp_token), 1);
      EXPECT_EQ(patch.count(if_token_2), 1);
      EXPECT_EQ(patch.count(if_cond_2), 1);
      EXPECT_EQ(patch.count(comp_2), 1);
      // level 4
      EXPECT_EQ(patch.count(comp_token_2), 1);
    }
  }

  // program 3
  // ------------------------------
  // int foo() {
  //   for (int i=0;i<c;i++) {
  //     a+=i;
  //   }
  // }
  // ------------------------------
  // - for
  // - FOR, init, cond, inc, comp
  // - {}, EXPR_STMT
  {
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[2];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);
    // level 1
    ASTNodeBase *for_stmt = matcher.getNodeByLoc("ForStmt", 2);
    // level 2
    ASTNodeBase *for_token = matcher.getNodeByLoc("TokenNode", 2);
    ASTNodeBase *init = matcher.getNodeByLoc("Expr", 2);
    ASTNodeBase *cond = matcher.getNodeByLoc("Expr", 2, 1);
    ASTNodeBase *inc = matcher.getNodeByLoc("Expr", 2, 2);
    ASTNodeBase *comp = matcher.getNodeByLoc("CompoundStmt", 2);
    // level 3
    ASTNodeBase *comp_token = matcher.getNodeByLoc("TokenNode", 2, 1);
    ASTNodeBase *expr_stmt = matcher.getNodeByLoc("ExprStmt", 3);
    {
      set<ASTNodeBase*> sel;
      sel.insert(for_token);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      ASSERT_EQ(patch.size(), 4);
      EXPECT_EQ(patch.count(for_stmt), 1);
      EXPECT_EQ(patch.count(for_token), 1);
      EXPECT_EQ(patch.count(comp), 1);
      EXPECT_EQ(patch.count(comp_token), 1);
    }
    {
      set<ASTNodeBase*> sel;
      sel.insert(expr_stmt);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      ASSERT_EQ(patch.size(), 1);
      // EXPECT_EQ(patch.count(for_stmt), 1);
      // EXPECT_EQ(patch.count(for_token), 1);
      // EXPECT_EQ(patch.count(comp), 1);
      // EXPECT_EQ(patch.count(comp_token), 1);
      EXPECT_EQ(patch.count(expr_stmt), 1);
    }
    {
      set<ASTNodeBase*> sel;
      sel.insert(inc);
      sel.insert(expr_stmt);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      // for (auto *node : patch) {
      //   node->dump(std::cout);
      //   std::cout << "\n";
      // }
      EXPECT_EQ(patch.size(), 4);
      EXPECT_EQ(patch.count(for_stmt), 1);
      EXPECT_EQ(patch.count(for_token), 1);
      EXPECT_EQ(patch.count(inc), 1);
      EXPECT_EQ(patch.count(comp), 0);
      EXPECT_EQ(patch.count(comp_token), 0);
      EXPECT_EQ(patch.count(expr_stmt), 1);

      // Generator gen;
      // gen.setSelection(patch);
      // unit->accept(&gen);
      // std::cout << gen.getProgram() << "\n";
    }
  }
  // Program 4
  // ------------------------------
  // int foo() {
  //   while (a<c) {
  //     a=b;
  //     b=c;
  //   }
  // }
  // ------------------------------
  // - while
  // - WHILE, cond, comp
  // - {}, exprstmt, exprstmt
  {
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[3];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);
    // level 1
    ASTNodeBase *while_stmt = matcher.getNodeByLoc("WhileStmt", 2);
    // level 2
    ASTNodeBase *while_token = matcher.getNodeByLoc("TokenNode", 2);
    ASTNodeBase *cond = matcher.getNodeByLoc("Expr", 2);
    ASTNodeBase *comp = matcher.getNodeByLoc("CompoundStmt", 2);
    // level 3
    ASTNodeBase *comp_token = matcher.getNodeByLoc("TokenNode", 2, 1);
    ASTNodeBase *expr_stmt = matcher.getNodeByLoc("ExprStmt", 3);
    ASTNodeBase *expr_stmt_2 = matcher.getNodeByLoc("ExprStmt", 4);
    ASSERT_TRUE(comp_token);
    ASSERT_TRUE(expr_stmt);
    ASSERT_TRUE(expr_stmt_2);
    {
      set<ASTNodeBase*> sel;
      sel.insert(cond);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      // for (auto *node : patch) {
      //   node->dump(std::cout);
      //   std::cout << "\n";
      // }
      EXPECT_EQ(patch.size(), 5);
      EXPECT_EQ(patch.count(while_stmt), 1);
      EXPECT_EQ(patch.count(while_token), 1);
      EXPECT_EQ(patch.count(cond), 1);
      EXPECT_EQ(patch.count(comp), 1);
      EXPECT_EQ(patch.count(comp_token), 1);
    }
    {
      set<ASTNodeBase*> sel;
      sel.insert(expr_stmt_2);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      EXPECT_EQ(patch.size(), 1);
      EXPECT_EQ(patch.count(expr_stmt_2), 1);
    }
  }
  // Program 6
  // ------------------------------
  // int foo(int a, int b) {      ... 1
  //   int c=8;
  //   while (a<c) {
  //     a=b;
  //     if (b>0) {
  //       d=c+e;      ... 6
  //     }
  //     b=c;
  //   }
  // }
  // ------------------------------
  // - 0: ..., comp
  // - {}, declstmt, while
  // - WHILE, cond, comp
  // - {}, exprstmt, if, exprstmt
  // - IF, cond, comp
  // - {}, exprstmt
  {
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[5];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);

    // Printer printer;
    // unit->accept(&printer);
    // std::cout << printer.getString() << "\n";

    // level 0
    ASTNodeBase *comp_0 = matcher.getNodeByLoc("CompoundStmt", 1);
    // level 1
    ASTNodeBase *comp_token_0 = matcher.getNodeByLoc("TokenNode", 1, 3);
    ASTNodeBase *decl = matcher.getNodeByLoc("DeclStmt", 2);
    ASTNodeBase *while_stmt = matcher.getNodeByLoc("WhileStmt", 3);
    // level 2
    ASTNodeBase *while_token = matcher.getNodeByLoc("TokenNode", 3);
    ASTNodeBase *while_cond = matcher.getNodeByLoc("Expr", 3);
    ASTNodeBase *comp = matcher.getNodeByLoc("CompoundStmt", 3);
    // level 3
    ASTNodeBase *comp_token = matcher.getNodeByLoc("TokenNode", 3, 1);
    ASTNodeBase *expr_stmt = matcher.getNodeByLoc("ExprStmt", 4);
    ASTNodeBase *if_stmt = matcher.getNodeByLoc("IfStmt", 5);
    ASTNodeBase *expr_stmt_2 = matcher.getNodeByLoc("ExprStmt", 8);
    // level 4
    ASTNodeBase *if_token = matcher.getNodeByLoc("TokenNode", 5);
    ASTNodeBase *if_cond = matcher.getNodeByLoc("Expr", 5);
    ASTNodeBase *comp_2 = matcher.getNodeByLoc("CompoundStmt", 5);
    // level 5
    ASTNodeBase *comp_token_2 = matcher.getNodeByLoc("TokenNode", 5, 1);
    ASTNodeBase *expr_stmt_3 = matcher.getNodeByLoc("ExprStmt", 6);
    {
      set<ASTNodeBase*> sel;
      // int c=8;
      // a = b;
      // should output their own
      sel.insert(decl);
      sel.insert(expr_stmt);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();

      // for (auto *node : patch) {
      //   node->dump(std::cout);
      //   std::cout << "\n";
      // }

      // Generator gen;
      // gen.setSelection(patch);
      // unit->accept(&gen);
      // std::cout << gen.getProgram() << "\n";
      
      EXPECT_EQ(patch.size(), 4);
      EXPECT_EQ(patch.count(comp_0), 1);
      EXPECT_EQ(patch.count(comp_token_0), 1);
      EXPECT_EQ(patch.count(decl), 1);
      EXPECT_EQ(patch.count(expr_stmt), 1);
    }
    {

      // std::cout << "======================" << "\n";
      set<ASTNodeBase*> sel;
      sel.insert(if_cond);
      sel.insert(while_cond);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      EXPECT_EQ(patch.size(), 8);
      EXPECT_EQ(patch.count(while_stmt), 1);
      EXPECT_EQ(patch.count(while_token), 1);
      EXPECT_EQ(patch.count(while_cond), 1);
      EXPECT_EQ(patch.count(comp), 0);
      EXPECT_EQ(patch.count(comp_token), 0);
      EXPECT_EQ(patch.count(if_stmt), 1);
      EXPECT_EQ(patch.count(if_token), 1);
      EXPECT_EQ(patch.count(if_cond), 1);
      EXPECT_EQ(patch.count(comp_2), 1);
      EXPECT_EQ(patch.count(comp_token_2), 1);

      // Generator gen;
      // gen.setSelection(patch);
      // unit->accept(&gen);
      // std::cout << gen.getProgram() << "\n";
    }
    {
      set<ASTNodeBase*> sel;
      sel.insert(if_token);
      sel.insert(decl);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      // for (auto *node : patch) {
      //   node->dump(std::cout);
      //   std::cout << "\n";
      // }
      EXPECT_EQ(patch.size(), 8);
      EXPECT_EQ(patch.count(comp_0), 1);
      EXPECT_EQ(patch.count(comp_token_0), 1);
      EXPECT_EQ(patch.count(decl), 1);
      EXPECT_EQ(patch.count(if_stmt), 1);
      EXPECT_EQ(patch.count(if_token), 1);
      EXPECT_EQ(patch.count(if_cond), 1);
      EXPECT_EQ(patch.count(comp_2), 1);
      EXPECT_EQ(patch.count(comp_token_2), 1);
    }
    {
      set<ASTNodeBase*> sel;
      sel.insert(if_token);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      EXPECT_EQ(patch.size(), 5);
      EXPECT_EQ(patch.count(if_stmt), 1);
      EXPECT_EQ(patch.count(if_token), 1);
      EXPECT_EQ(patch.count(if_cond), 1);
      EXPECT_EQ(patch.count(comp_2), 1);
      EXPECT_EQ(patch.count(comp_token_2), 1);
    }
  }
  // Program 7
  // ------------------------------
  // int foo() {
  //   int a=8,b=9;
  //   switch (a) {
  //   case 1: a=9; b=10; break;
  //   case 2: {a=b+1;}
  //   default: break;
  //   }
  // }
  // ------------------------------
  // - declstmt, switch
  // - SWITCH, cond, case, case, default
  // - CASE, cond, exprstmt, exprstmt, break, CASE, cond, comp, DEFAULT, break
  // - {}, exprstmt
  {
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[6];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);
    // level 0
    ASTNodeBase *comp_0 = matcher.getNodeByLoc("CompoundStmt", 1);
    // level 1
    ASTNodeBase *comp_token_0 = matcher.getNodeByLoc("TokenNode", 1, 3);
    ASTNodeBase *decl = matcher.getNodeByLoc("DeclStmt", 2);
    ASTNodeBase *switch_stmt = matcher.getNodeByLoc("SwitchStmt", 3);
    // level 2
    ASTNodeBase *switch_token = matcher.getNodeByLoc("TokenNode", 3);
    ASTNodeBase *switch_cond = matcher.getNodeByLoc("Expr", 3);
    ASTNodeBase *case_1 = matcher.getNodeByLoc("CaseStmt", 4);
    ASTNodeBase *case_2 = matcher.getNodeByLoc("CaseStmt", 5);
    ASTNodeBase *default_stmt = matcher.getNodeByLoc("DefaultStmt", 6);
    // level 3
    ASTNodeBase *case_token = matcher.getNodeByLoc("TokenNode", 4);
    ASTNodeBase *case_cond = matcher.getNodeByLoc("Expr", 4);
    ASTNodeBase *expr_stmt = matcher.getNodeByLoc("ExprStmt", 4);
    ASTNodeBase *expr_stmt_2 = matcher.getNodeByLoc("ExprStmt", 4, 1);
    ASTNodeBase *break_stmt = matcher.getNodeByLoc("BreakStmt", 4);
    ASTNodeBase *case_token_2 = matcher.getNodeByLoc("TokenNode", 5);
    ASTNodeBase *case_cond_2 = matcher.getNodeByLoc("Expr", 5);
    ASTNodeBase *comp = matcher.getNodeByLoc("CompoundStmt", 5);
    ASTNodeBase *default_token = matcher.getNodeByLoc("TokenNode", 6);
    ASTNodeBase *break_stmt_2 = matcher.getNodeByLoc("BreakStmt", 6);
    // level 4
    ASTNodeBase *comp_token = matcher.getNodeByLoc("TokenNode", 5, 2);
    ASTNodeBase *expr_stmt_3 = matcher.getNodeByLoc("ExprStmt", 5);
    {
      // switch
      // output:
      // switch(a) {}
      set<ASTNodeBase*> sel;
      sel.insert(switch_token);
      StandAloneGrammarPatcher patcher(ast, sel);
      patcher.process();
      set<ASTNodeBase*> patch = patcher.getPatch();
      EXPECT_EQ(patch.size(), 3);
      EXPECT_EQ(patch.count(switch_stmt), 1);
      EXPECT_EQ(patch.count(switch_token), 1);
      EXPECT_EQ(patch.count(switch_cond), 1);

      // Generator gen;
      // gen.setSelection(patch);
      // unit->accept(&gen);
      // std::cout << gen.getProgram() << "\n";
    }
  }
}

TEST(SymbolTableTest, DeclStmtTest) {
  const char *program = R"prefix(
int foo() {
  int a;
  int b,c=10;
  while (a<c) {
    a=b;
    if (b>0) {
      a=c;
    } else {
      a=8;
    }
  }
}
)prefix";
  std::string prog = program;
  utils::trim(prog);

  fs::path dir = fs::temp_directory_path();
  fs::create_directories(dir);

  fs::path source = dir / "a.c";
  std::ofstream of (source.string());
  ASSERT_TRUE(of.is_open());
  of << prog;
  of.close();
  Parser *parser = new Parser(source.string());
  ASTContext *ast = parser->getASTContext();

  ParentIndexer indexer;
  Matcher matcher;
  TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
  unit->accept(&indexer);
  unit->accept(&matcher);

  ASTNodeBase *decl_a = matcher.getNodeByLoc("DeclStmt", 2);
  ASTNodeBase *decl_bc = matcher.getNodeByLoc("DeclStmt", 3);
  ASTNodeBase *while_cond = matcher.getNodeByLoc("Expr", 4);
  ASTNodeBase *expr_stmt = matcher.getNodeByLoc("ExprStmt", 5);
  ASTNodeBase *if_cond = matcher.getNodeByLoc("Expr", 6);
  ASTNodeBase *expr_stmt_2 = matcher.getNodeByLoc("ExprStmt", 7);
  ASTNodeBase *expr_stmt_3 = matcher.getNodeByLoc("ExprStmt", 9);
    
  SymbolTableBuilder builder;
  unit->accept(&builder);
  std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > u2d = builder.getUse2DefMap();

  // for (auto m : u2d) {
  //   m.first->dump(std::cout);
  //   std::cout << " ==> ";
  //   for (auto *node : m.second) {
  //     node->dump(std::cout);
  //   }
  //   std::cout << "\n";
  // }

  EXPECT_EQ(u2d.size(), 5);
  // while (a<c)
  ASSERT_EQ(u2d.count(while_cond), 1);
  EXPECT_EQ(u2d[while_cond].size(), 2);
  EXPECT_EQ(u2d[while_cond].count(decl_a), 1);
  EXPECT_EQ(u2d[while_cond].count(decl_bc), 1);
  // a=b
  ASSERT_EQ(u2d.count(expr_stmt), 1);
  EXPECT_EQ(u2d[expr_stmt].size(), 2);
  EXPECT_EQ(u2d[expr_stmt].count(decl_a), 1);
  EXPECT_EQ(u2d[expr_stmt].count(decl_bc), 1);
  // b>0
  ASSERT_EQ(u2d.count(if_cond), 1);
  EXPECT_EQ(u2d[if_cond].size(), 1);
  EXPECT_EQ(u2d[if_cond].count(decl_bc), 1);
  // a=c
  ASSERT_EQ(u2d.count(expr_stmt_2), 1);
  EXPECT_EQ(u2d[expr_stmt_2].size(), 2);
  EXPECT_EQ(u2d[expr_stmt_2].count(decl_a), 1);
  EXPECT_EQ(u2d[expr_stmt_2].count(decl_bc), 1);
  // a=8
  ASSERT_EQ(u2d.count(expr_stmt_3), 1);
  EXPECT_EQ(u2d[expr_stmt_3].size(), 1);
  EXPECT_EQ(u2d[expr_stmt_3].count(decl_a), 1);
}

TEST(SymbolTableTest, ParamTest) {
  const char *program = R"prefix(
int foo(int a, int b) {
  while (a<b) {
    a=b;
    if (b>0) {
      b=a;
    }
  }
}
)prefix";
  std::string prog = program;
  utils::trim(prog);

  fs::path dir = fs::temp_directory_path();
  fs::create_directories(dir);

  fs::path source = dir / "a.c";
  std::ofstream of (source.string());
  ASSERT_TRUE(of.is_open());
  of << prog;
  of.close();
  Parser *parser = new Parser(source.string());
  ASTContext *ast = parser->getASTContext();

  ParentIndexer indexer;
  Matcher matcher;
  TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
  unit->accept(&indexer);
  unit->accept(&matcher);

  ASTNodeBase *function_decl = matcher.getNodeByLoc("FunctionDecl", 1);
  // a<b
  ASTNodeBase *while_cond = matcher.getNodeByLoc("Expr", 2);
  // a=b
  ASTNodeBase *expr_stmt = matcher.getNodeByLoc("ExprStmt", 3);
  // b>0
  ASTNodeBase *if_cond = matcher.getNodeByLoc("Expr", 4);
  // b=a
  ASTNodeBase *expr_stmt_2 = matcher.getNodeByLoc("ExprStmt", 5);
    
  SymbolTableBuilder builder;
  unit->accept(&builder);
  std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > u2d = builder.getUse2DefMap();
  EXPECT_EQ(u2d.size(), 4);

  ASSERT_EQ(u2d.count(while_cond), 1);
  EXPECT_EQ(u2d[while_cond].count(function_decl), 1);
  ASSERT_EQ(u2d.count(expr_stmt), 1);
  EXPECT_EQ(u2d[expr_stmt].count(function_decl), 1);
  ASSERT_EQ(u2d.count(if_cond), 1);
  EXPECT_EQ(u2d[if_cond].count(function_decl), 1);
  ASSERT_EQ(u2d.count(expr_stmt_2), 1);
  EXPECT_EQ(u2d[expr_stmt_2].count(function_decl), 1);
}

TEST(SymbolTableTest, ForTest) {
  const char *program = R"prefix(
int foo() {
  int c=8;
  int i=7;
  for (int i=0;i<c;i++) {
    i++;
  }
}
)prefix";
  std::string prog = program;
  utils::trim(prog);

  fs::path dir = fs::temp_directory_path();
  fs::create_directories(dir);

  fs::path source = dir / "a.c";
  std::ofstream of (source.string());
  ASSERT_TRUE(of.is_open());
  of << prog;
  of.close();
  Parser *parser = new Parser(source.string());
  ASTContext *ast = parser->getASTContext();

  ParentIndexer indexer;
  Matcher matcher;
  TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
  unit->accept(&indexer);
  unit->accept(&matcher);

  ASTNodeBase *decl_c = matcher.getNodeByLoc("DeclStmt", 2);
  ASTNodeBase *decl_i = matcher.getNodeByLoc("DeclStmt", 3);
  ASTNodeBase *for_init = matcher.getNodeByLoc("Expr", 4);
  ASTNodeBase *for_cond = matcher.getNodeByLoc("Expr", 4, 1);
  ASTNodeBase *for_inc = matcher.getNodeByLoc("Expr", 4, 2);
  ASTNodeBase *for_body = matcher.getNodeByLoc("ExprStmt", 5);
  
  ASSERT_TRUE(decl_c);
  ASSERT_TRUE(decl_i);
  ASSERT_TRUE(for_init);
  ASSERT_TRUE(for_cond);
  ASSERT_TRUE(for_inc);
  ASSERT_TRUE(for_body);

  // std::cout << "for init: " << "\n";
  // for_init->dump(std::cout);
  // std::cout << "For cond: " << "\n";
  // for_cond->dump(std::cout);
  // std::cout << "For inc: " << "\n";
  // for_inc->dump(std::cout);
    
  SymbolTableBuilder builder;
  unit->accept(&builder);
  std::map<v2::ASTNodeBase*,std::set<v2::ASTNodeBase*> > u2d = builder.getUse2DefMap();
  // for (auto &m : u2d) {
  //   m.first->dump(std::cout);
  //   std::cout << " ==> ";
  //   for (auto *node : m.second) {
  //     node->dump(std::cout);
  //   }
  //   std::cout << "\n";
  // }
  
  EXPECT_EQ(u2d.size(), 3);

  ASSERT_EQ(u2d.count(for_cond), 1);
  EXPECT_EQ(u2d[for_cond].count(decl_i), 0);
  EXPECT_EQ(u2d[for_cond].count(for_init), 1);
  EXPECT_EQ(u2d[for_cond].count(decl_c), 1);
  ASSERT_EQ(u2d.count(for_inc), 1);
  EXPECT_EQ(u2d[for_inc].count(decl_i), 0);
  EXPECT_EQ(u2d[for_inc].count(for_init), 1);
  ASSERT_EQ(u2d.count(for_body), 1);
  EXPECT_EQ(u2d[for_inc].count(decl_i), 0);
  EXPECT_EQ(u2d[for_inc].count(for_init), 1);
}

// TEST_F(VisitorTest, DistributorTest) {
// }

// TEST_F(VisitorTest, GeneratorTest) {
// }






// TEST_F(VisitorTest, SimplePreorderVisitorTest) {
//   {
//     SimplePreorderVisitor visitor;
//     TranslationUnitDecl *unit = asts[0]->getTranslationUnitDecl();
//     unit->accept(&visitor);
//     vector<ASTNodeBase*> nodes = visitor.getNodes();
//     // for (ASTNodeBase *node : nodes) {
//     //   node->dump(std::cout);
//     // }

//     vector<string> expect = {
//       "TranslationUnitDecl", // UNIT
//       "FunctionDecl", // FUNC
//       "TokenNode", "TokenNode", "TokenNode", // int, foo, ()
//       "CompStmt",
//       "IfStmt", // IF
//       "TokenNode", "Expr", // if
//       "CompStmt",
//       "ExprStmt" // d=c+e
//     };

//     ASSERT_EQ(nodes.size(), expect.size());
//     for (int i=0;i<nodes.size();i++) {
//       EXPECT_EQ(nodes[i]->getNodeName(), expect[i]);
//     }
//   }
//   {
//     SimplePreorderVisitor visitor;
//     TranslationUnitDecl *unit = asts[1]->getTranslationUnitDecl();
//     unit->accept(&visitor);
//     vector<ASTNodeBase*> nodes = visitor.getNodes();
    
//     vector<string> expect = {
//       "TranslationUnitDecl", // UNIT
//       "FunctionDecl", // FUNC
//       "TokenNode", "TokenNode", "TokenNode", // int, foo, ()
//       "CompStmt",
//       "IfStmt", // IF
//       "TokenNode", "Expr", // if
//       "CompStmt",
//       "ExprStmt", // d=c+e
//       "TokenNode", // else
//       "IfStmt", // IF
//       "TokenNode", "Expr", // if
//       "CompStmt",
//       "ExprStmt" // a=8
//     };

//     ASSERT_EQ(nodes.size(), expect.size());
//     for (int i=0;i<nodes.size();i++) {
//       EXPECT_EQ(nodes[i]->getNodeName(), expect[i]);
//     }
//   }
// }
