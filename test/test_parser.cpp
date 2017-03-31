#include "helium/parser/parser.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "helium/parser/GrammarPatcher.h"

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
      of << programs[i];
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
    vector<ASTNodeBase*> v;
    v = matcher.match("TranslationUnitDecl/FunctionDecl/CompStmt/IfStmt");
    // std::cout << matcher.size() << "\n";
    // matcher.dump(std::cout);
    ASSERT_EQ(v.size(), 1);
    EXPECT_EQ(visitor.getLevel(v[0]), 3);
    v = matcher.match("TranslationUnitDecl/FunctionDecl/CompStmt/IfStmt/Expr");
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
    v = matcher.match("TranslationUnitDecl/FunctionDecl/CompStmt/IfStmt");
    // std::cout << matcher.size() << "\n";
    // matcher.dump(std::cout);
    ASSERT_EQ(v.size(), 1);
    ASTNodeBase *comp = indexer.getParent(v[0]);
    ASSERT_TRUE(comp);
    EXPECT_EQ(comp->getNodeName(), "CompStmt");
    ASTNodeBase *func = indexer.getParent(comp);
    ASSERT_TRUE(func);
    EXPECT_EQ(func->getNodeName(), "FunctionDecl");
    vector<ASTNodeBase*> children = indexer.getChildren(func);
    EXPECT_EQ(children.size(), 4); // int, foo, (), comp
    EXPECT_NE(std::find(children.begin(), children.end(), comp), children.end());
    children = indexer.getChildren(comp);
    EXPECT_EQ(children.size(), 1);
    children = indexer.getChildren(v[0]);
    // for (auto *c : children) {
    //   c->dump(std::cout);
    // }
    // std::cout  << "\n";
    ASSERT_EQ(children.size(), 3);
    EXPECT_EQ(children[0]->getNodeName(), "TokenNode");
    EXPECT_EQ(children[1]->getNodeName(), "Expr");
    EXPECT_EQ(children[2]->getNodeName(), "CompStmt");
    // EXPECT_EQ(children[3]->getNodeName(), "TokenNode");
    // EXPECT_EQ(children[4]->getNodeName(), "CompStmt");
  }
}

TEST_F(VisitorTest, GrammarPatcherTest) {
  {
    // set up indexer and matcher for selecting nodes
    ParentIndexer indexer;
    Matcher matcher;
    ASTContext *ast = asts[0];
    TranslationUnitDecl *unit = ast->getTranslationUnitDecl();
    unit->accept(&indexer);
    unit->accept(&matcher);

    {
      // create selection
      // if token
      set<ASTNodeBase*> sel;
      vector<ASTNodeBase*> v;
      v = matcher.match("TranslationUnitDecl/FunctionDecl/CompStmt/IfStmt/TokenNode");
      ASSERT_GT(v.size(), 0);
      sel.insert(v[0]);

      v[0]->dump(std::cout);
      
      // create GrammarPatcher
      StandAloneGrammarPatcher patcher(ast, sel);
      set<ASTNodeBase*> patch = patcher.getPatch();
      EXPECT_EQ(patch.size(), 3);
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
