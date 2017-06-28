#include "helium/parser/ast.h"
#include "helium/parser/xml_doc_reader.h"
#include "helium/utils/helium_options.h"
#include "helium/parser/ast_node.h"

/**
 * print the AST associated with the file
 */
int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Need to provide source file name\n");
    exit(1);
  }

  std::cout << "Running AST Printer" << "\n";

  HeliumOptions::Instance()->ParseConfigFile("~/.heliumrc");
  
  XMLDoc *doc = XMLDocReader::CreateDocFromFile(argv[1]);
  XMLNode node = doc->document_element();
  AST *ast = new AST(node);
  // ast->dump();
  std::cout << ast->GetCode() << "\n";
  ASTNode *root = ast->GetRoot();
  std::cout << root->GetLabel() << "\n";
  

  std::cout << "End" << "\n";
  return 0;
}
