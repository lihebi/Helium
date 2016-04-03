#include "ast_node.h"
#include "utils.h"
#include "ast.h"
#include <gtest/gtest.h>

using namespace ast;


/**
 * Disable because it will not check something.
 * It just run some code, visualize it.
 */
TEST(ASTNodeTestCase, DISABLED_NodeTest) {
  ast::Doc doc;
const char *raw = R"prefix(

int foo() {
if (x>0) {
  while (x<10) {
    a=b;
    c=d;
    if (a>c) {
      sum+=c;
    } else if (a==c) {
      sum += con1;
    } else {
      sum += a;
    }
  }
} else {
  sum = 0;
  for (int i=0;i<8;i++) {
    sum += i;
  }
}
}

)prefix";

 utils::string2xml(raw, doc);
 NodeList nodes = find_nodes(doc, NK_Function);
 ASSERT_EQ(nodes.size(), 1);
 AST *ast = new AST(nodes[0]);
 std::string code = ast->GetCode();
 // code = utils::exec_in("indent", code.c_str());
 std::cout <<code  << "\n";
// THIS IS IMPORTANT! SEED the random.
 utils::seed_rand();
 Gene gene, cgene;
 std::string dot;

 std::cout <<"begin test suite"  << "\n";

 /**
  * Suite 1
  */
 gene.Rand(ast->size());
 // gene.SetFlat("0000111011101010010110000");
 std::cout <<"gene: ";
 gene.dump();
 cgene = ast->CompleteGene(gene);
 std::cout <<"cgene: ";
 cgene.dump();
 dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
 utils::visualize_dot_graph(dot);

 /**
  * Suite 2
  */
 // gene.Rand(ast->size());
 // std::cout <<"gene: ";
 // gene.dump();
 // cgene = ast->CompleteGene(gene);
 // std::cout <<"cgene: ";
 // cgene.dump();
 // dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
 // utils::visualize_dot_graph(dot);

 // std::cout <<dot  << "\n";
 delete ast;
}

TEST(ASTNodeTestCase, DISABLED_ExtraNodeTest) {
  ast::Doc doc;
  const char *raw = R"prefix(
int
res_hnok(const char *dn) {
	int ppch = '\0', pch = PERIOD, ch = *dn++;

	while (ch != '\0') {
		int nch = *dn++;

		if (periodchar(ch)) {
			(void)NULL;
		} else if (periodchar(pch)) {
			if (!borderchar(ch))
				return (0);
		} else if (periodchar(nch) || nch == '\0') {
			if (!borderchar(ch))
				return (0);
		} else {
			if (!middlechar(ch))
				return (0);
		}
		ppch = pch, pch = ch, ch = nch;
	}
	return (1);
}
)prefix";

  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 1);
  AST *ast = new AST(nodes[0]);
  ast->Visualize();
}

TEST(ASTNodeTestCase, DISABLED_VarDefUseTest) {
  ast::Doc doc;
  const char *raw = R"prefix(


int foo() {
  int x=0;
  int sum=0;
  if (x>0) {
    int a=1;
    int b=1;
    int c=2;
    int d=3;
    while (x<10) {
      a=b;
      c=d;
      int con1=8;
      if (a>c) {
        sum+=c;
      } else if (a==c) {
        sum += con1;
      } else {
        sum += a;
      }
    }
  } else {
    sum = 0;
    for (int i=0;i<8;i++) {
      sum += i;
    }
  }
  return sum;
}
)prefix";

  utils::string2xml(raw, doc);
  NodeList nodes = find_nodes(doc, NK_Function);
  ASSERT_EQ(nodes.size(), 1);
  AST *ast = new AST(nodes[0]);
  std::string code = ast->GetCode();
  // code = utils::exec_in("indent", code.c_str());
  std::cout <<code  << "\n";
  // THIS IS IMPORTANT! SEED the random.
  utils::seed_rand();
  Gene gene, cgene;
  std::string dot;

  std::cout <<"begin test suite"  << "\n";

  /**
   * Suite 1
   */
  gene.Rand(ast->size());
  std::cout <<"gene: ";
  gene.dump();
  // cgene = ast->CompleteGene(gene);
  // gene.SetFlat("10000110000001010100010010000000");

  // deprecated
  // cgene = ast->CompleteVarDefUse(gene);
  std::cout <<"cgene: ";
  cgene.dump();
  dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
  // utils::visualize_dot_graph(dot);

  /**
   * Suite 2
   */
  // gene.Rand(ast->size());
  // std::cout <<"gene: ";
  // gene.dump();
  // cgene = ast->CompleteGene(gene);
  // std::cout <<"cgene: ";
  // cgene.dump();
  // dot = ast->VisualizeI(gene.GetIndiceS(), cgene.GetIndiceS());
  // utils::visualize_dot_graph(dot);

  // std::cout <<dot  << "\n";
  delete ast;
}
