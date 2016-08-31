#include "type.h"
#include "type_helper.h"
#include "utils/utils.h"
#include "config/config.h"
#include "corner.h"

/********************************
 * IntType
 *******************************/

IntType::IntType() {}

IntType::~IntType() {}

std::string IntType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// IntType::GetDeclCode: " + var + "\n";
  ret += "int " + var + ";\n";
  return ret;
}

std::string IntType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// IntType::GetInputCode: " + var + "\n";
  ret += get_scanf_code("%d", "&" + var);
  return ret;
}

std::string IntType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// IntType::GetOutputCode: " + var + "\n";
  ret += "printf(\"Od_" + var + " = " + "%d" + "\\n\", " + var + ");\n";
  ret += flush_output;
  return ret;
}

InputSpec *IntType::GenerateInput() {
  static int int_min = Config::Instance()->GetInt("int-min");
  static int int_max = Config::Instance()->GetInt("int-max");
  static int max_array_size = Config::Instance()->GetInt("max-array-size");
  assert(max_array_size > 0);

  int value = utils::rand_int(int_min, int_max);
  std::string spec = "{int: " + std::to_string(value) + "}";
  std::string raw = std::to_string(value);
  return new InputSpec(spec, raw);
}

InputSpec *IntType::wrap(int value) {
  std::string spec = "{int: " + std::to_string(value) + "}";
  std::string raw = std::to_string(value);
  return new InputSpec(spec, raw);
}

int IntType::corner() {
  std::set<int> choices = {-2, -1, 0, 1, 2, 10, 100, 100};
  for (int i=0;i<10;i++) {
    choices.insert(pow(2, i));
  }
  choices.insert(BUFSIZ);
  // TODO literal values from program
  std::set<int> additional;
  for (int a : choices) {
    additional.insert(a-2);
    additional.insert(a-1);
    additional.insert(a+1);
    additional.insert(a+2);
  }
  choices.insert(additional.begin(), additional.end());
  std::vector<int> v(choices.begin(), choices.end());
  // choose from choices
  int r = utils::rand_int(0, v.size()-1);
  return v[r];
}


/**
 * Generate all the inputs used for pairwise testing
 * 1. some corner cases
 * 2. some random values: 5
 */
std::vector<InputSpec*> IntType::GeneratePairInput() {
  std::vector<InputSpec*> ret;
  std::vector<int> corners = Corner::Instance()->GetIntCorner();
  ret.push_back(GenerateInput());
  for (int corner : corners) {
    ret.push_back(wrap(corner));
  }
  return ret;
}


/**
 * Bool
 */

BoolType::BoolType() {}

BoolType::~BoolType() {}

std::string BoolType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// BoolType::GetDeclCode: " + var + "\n";
  ret += "bool " + var + ";\n";
  return ret;
}

std::string BoolType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// BoolType::GetInputCode: " + var + "\n";
  ret += "scanf(\"%d\", &" + var + ");\n";
  get_helium_size_branch(var + " = false",
                         var + " = true"
                         );
  return ret;
}

std::string BoolType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// BoolType::GetOutputCode: " + var + "\n";
  ret += "printf(\"Ob_" + var + " = " + "%d" + "\\n\", " + var + ");\n";
  ret += flush_output;
  return ret;
}

InputSpec *BoolType::GenerateInput() {
  // static int int_min = Config::Instance()->GetBool("int-min");
  // static int int_max = Config::Instance()->GetBool("int-max");
  // static int max_array_size = Config::Instance()->GetBool("max-array-size");
  // assert(max_array_size > 0);

  // int value = utils::rand_int(int_min, int_max);
  bool b = utils::rand_bool();
  std::string spec = "{bool: " + std::to_string(b) + "}";
  std::string raw = std::to_string(b);
  return new InputSpec(spec, raw);
}



/********************************
 * CharType
 *******************************/

CharType::CharType() {}

CharType::~CharType() {}

std::string CharType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// CharType::GetDeclCode: " + var + "\n";
  ret += "char " + var + ";\n";
  return ret;
}

std::string CharType::GetInputCode(std::string var) {
  std::string ret;
  ret += get_scanf_code("%c", "&" + var);
  return ret;
}

std::string CharType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// CharType::GetOutputCode: " + var + "\n";
  ret += "printf(\"Oc_" + var + " = %c\\n\", " + var + ");\n" + flush_output;
  return ret;
}

InputSpec *CharType::GenerateInput() {
  char c = utils::rand_char();
  std::string spec = "{char: " + std::to_string(c) + "}";
  std::string raw = std::to_string(c);
  return new InputSpec(spec, raw);
}

char CharType::corner() {
  std::vector<char> choices = {'\0', '\t', '\v', '\n'};
  int r = utils::rand_int(0, choices.size()-1);
  return choices[r];
}

InputSpec *CharType::wrap(char c) {
  std::string spec = "{char: " + std::to_string(c) + "}";
  std::string raw = std::to_string(c);
  return new InputSpec(spec, raw);
}

std::vector<InputSpec*> CharType::GeneratePairInput() {
  std::vector<InputSpec*> ret;
  std::vector<char> corners = Corner::Instance()->GetCharCorner();
  for (char c : corners) {
    ret.push_back(wrap(c));
  }
  for (int i=0;i<5;i++) {
    ret.push_back(GenerateInput());
  }
  return ret;
}
