#include "type.h"
#include "common.h"
#include "io_helper.h"

#include "parser/xml_doc_reader.h"
#include "parser/ast_node.h"

#include "utils/utils.h"
#include <gtest/gtest.h>


static void get_instrument_code(std::string code, std::string &input, std::string &output) {
  // std::string code = "char *var;";
  XMLDoc *doc = XMLDocReader::CreateDocFromString(code);
  XMLNode decl_node = find_first_node_bfs(doc->document_element(), "decl");
  Decl *decl = DeclFactory::CreateDecl(decl_node);
  Type *type = decl->GetType();
  std::string var = decl->GetName();
  output = type->GetOutputCode(var);
  input = type->GetInputCode(var);
  utils::trim(input);
  utils::trim(output);
}

static bool equal_each_line(std::string s1, std::string s2) {
  std::vector<std::string> v1 = utils::split(s1, '\n');
  std::vector<std::string> v2 = utils::split(s2, '\n');
  int i1=0,i2=0;
  while (i1 < (int)v1.size() && i2 < (int)v2.size()) {
    std::string line1 = v1[i1];
    std::string line2 = v2[i2];
    utils::trim(line1);
    utils::trim(line2);
    if (line1.empty() || line1[0] == '/') {
      i1++; continue;
    }
    if (line2.empty() || line2[0] == '/') {
      i2++; continue;
    }
    // do the comparison
    if (line1 != line2) {
      std::cout << "Not Equal: " << "\n";
      std::cout << i1 << " <<<<<" << "\n";
      std::cout << line1 << "\n";
      std::cout << i2 << " >>>>>" << "\n";
      std::cout << line2 << "\n";
      return false;
    }
    i1++;i2++;
  }
  while (i1 < (int)v1.size()) {
    std::string line = v1[i1];
    utils::trim(line);
    if (!line.empty() && line[0] != '/') {
      std::cout << "v1 contain more lines: " << line << "\n";
      return false;
    }
    i1++;
  }
  while (i2 < (int)v2.size()) {
    std::string line = v2[i2];
    utils::trim(line);
    if (!line.empty() && line[0] != '/') {
      std::cout << "v2 Contain more lines: " << line << "\n";
      return false;
    }
    i2++;
  }
  return true;
}


TEST(TypeCase, TypeTest) {

  std::string code;


  std::vector<std::vector<std::string> > data = {
    {"char var;", "src/type/_test-data-char-var.c"},
    {"char *var;", "src/type/_test-data-char-star-var.c"},
    {"char **var;", "src/type/_test-data-char-star-star-var.c"}
  };

  // std::string output = utils::exec("pwd");
  // std::cout << output << "\n";

  // std::string content = utils::read_file("src/type/_test-data-char-var.c");
  // std::cout << content << "\n";

  for (auto v : data) {
    std::string code = v[0];

    // std::cout << utils::RED << code << utils::RESET << "\n";

    
    std::string file = v[1];
    std::string input, output;
    std::string expect_input, expect_output;
    std::string expect;
    get_instrument_code(code, input, output);

    // std::cout << "Reading input .." << "\n";

    expect = utils::read_file(file);
    size_t input_begin_pos = expect.find("// HELIUM_INPUT_BEGIN");
    ASSERT_TRUE(input_begin_pos != std::string::npos);
    input_begin_pos = expect.find('\n', input_begin_pos);
    ASSERT_TRUE(input_begin_pos != std::string::npos);
    size_t input_end_pos = expect.find("// HELIUM_INPUT_END");
    ASSERT_TRUE(input_end_pos != std::string::npos);
    expect_input = expect.substr(input_begin_pos, input_end_pos - input_begin_pos);

    // std::cout << "===" << "\n";
    // std::cout << expect_input << "\n";
    // std::cout << "===" << "\n";

    utils::trim(expect_input);
    
    // std::cout << "Reading output .."<< "\n";

    size_t output_begin_pos = expect.find("// HELIUM_OUTPUT_BEGIN");
    ASSERT_TRUE(output_begin_pos != std::string::npos);
    output_begin_pos = expect.find('\n', output_begin_pos);
    ASSERT_TRUE(output_begin_pos != std::string::npos);
    size_t output_end_pos = expect.find("// HELIUM_OUTPUT_END");
    ASSERT_TRUE(output_end_pos != std::string::npos);
    expect_output = expect.substr(output_begin_pos, output_end_pos - output_begin_pos);
    utils::trim(expect_output);

    // std::cout << "Asserting .." << "\n";

    EXPECT_TRUE(equal_each_line(expect_input, input));
    EXPECT_TRUE(equal_each_line(expect_output, output));
  }
}


TEST(TypeCase, NewTest) {
  TypeFactory::CreateType("int");
  std::string input,output;
  get_instrument_code("char *a;", input, output);
  std::cout << "input:" << "\n";
  std::cout << input << "\n";
  std::cout << "output:" << "\n";
  std::cout << output << "\n";
  std::cout << IOHelper::Instance()->GetInput("int") << "\n";
  std::cout << IOHelper::Instance()->GetOutput("int") << "\n";
  std::cout << "all" << "\n";
  std::cout << IOHelper::Instance()->GetAll() << "\n";
}
