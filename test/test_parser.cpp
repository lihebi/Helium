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
      ASSERT_EQ(patch.size(), 6);
      EXPECT_EQ(patch.count(for_stmt), 1);
      EXPECT_EQ(patch.count(for_token), 1);
      EXPECT_EQ(patch.count(inc), 1);
      EXPECT_EQ(patch.count(comp), 1);
      EXPECT_EQ(patch.count(comp_token), 1);
      EXPECT_EQ(patch.count(expr_stmt), 1);
    }
  }
}

// TEST_F(VisitorTest, SymbolTableBuilderTest) {
// }

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
