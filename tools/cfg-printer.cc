#include "helium/parser/ast_v2.h"
#include "helium/parser/parser.h"
#include "helium/parser/visitor.h"
#include <iostream>

using namespace v2;
/**
 * print the AST associated with the file
 */
int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Need to provide source file name\n");
    exit(1);
  }

  std::cout << "Running CFG Printer" << "\n";

  std::string file(argv[1]);
  // file = "/home/hebi/.helium.d/cache/_home_hebi_data_unziped_Dunedan--mbp-2016-linux/cpp/touchbar.c";
  Parser *parser = new Parser(file);
  ASTContext *ast = parser->getASTContext();
  TranslationUnitDecl *unit = ast->getTranslationUnitDecl();

  // unit->dump(std::cout);

  CFGBuilder builder;
  unit->accept(&builder);
  v2::CFG *cfg = builder.getCFG();
  std::string output_file = cfg->visualize();

  std::cout << "PNG file outputed to " << output_file << "\n";

  // agg
  std::string agg_filename = cfg->visualizeAgg();
  std::cout << "AGG file outputed to " << agg_filename << "\n";

  std::cout << "End" << "\n";
  return 0;
}
