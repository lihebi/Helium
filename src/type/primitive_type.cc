#include "helium/type/primitive_type.h"
#include "helium/type/type_helper.h"
#include "helium/utils/utils.h"
#include "helium/utils/helium_options.h"
#include "helium/type/corner.h"
#include "helium/type/io_helper.h"

/********************************
 * IntType
 *******************************/

IntType::IntType() {
  // construct InputSpec for all of the corners
  // put them into m_corners
  std::vector<int> corners = Corner::Instance()->GetIntCorner();
  for (int corner : corners) {
    m_corners.push_back(wrap(corner));
  }
}

IntType::~IntType() {}

std::string IntType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// IntType::GetDeclCode: " + var + "\n";
  ret += "int " + var + ";\n";
  return ret;
}

std::string IntType::GetInputCode(std::string var) {
  // return "input_int(&" + var + ");\n";
  return IOHelper::GetInputCall("int", var);
}

std::string IntType::GetOutputCode(std::string var) {
  std::string ret;
  return IOHelper::GetOutputCall("int", var, var);
  // return "output_int("+var+",\"" + var + "\");\n";
}

InputSpec *IntType::GenerateRandomInput() {
  static int int_min = HeliumOptions::Instance()->GetInt("test-input-min-int");
  static int int_max = HeliumOptions::Instance()->GetInt("test-input-max-int");
  static int max_array_size = HeliumOptions::Instance()->GetInt("test-input-max-array-size");
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
/**
 * Generate all the inputs used for pairwise testing
 * 1. some corner cases
 * 2. some random values: 5
 */
std::vector<InputSpec*> IntType::GeneratePairInput() {
  std::vector<InputSpec*> ret;
  std::vector<int> corners = Corner::Instance()->GetIntCorner();
  ret.push_back(GenerateRandomInput());
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
  return IOHelper::GetInputCall("bool", var);
  // return "input_bool(&" + var + ");\n";
}

std::string BoolType::GetOutputCode(std::string var) {
  return IOHelper::GetOutputCall("bool", var, var);
  // return "output_bool("+var+",\"" + var + "\");\n";
}

InputSpec *BoolType::GenerateRandomInput() {
  bool b = utils::rand_bool();
  std::string spec = "{bool: " + std::to_string(b) + "}";
  std::string raw = std::to_string(b);
  return new InputSpec(spec, raw);
}



/********************************
 * CharType
 *******************************/

CharType::CharType() {
  std::vector<char> corners = Corner::Instance()->GetCharCorner();
  for (char c : corners) {
    m_corners.push_back(wrap(c));
  }
}

CharType::~CharType() {}

std::string CharType::GetDeclCode(std::string var) {
  std::string ret;
  ret += "// CharType::GetDeclCode: " + var + "\n";
  ret += "char " + var + ";\n";
  return ret;
}

std::string CharType::GetInputCode(std::string var) {
  return IOHelper::GetInputCall("char", var);
  // return "input_char(&"+var+");\n";
}

std::string CharType::GetOutputCode(std::string var) {
  return IOHelper::GetOutputCall("char", var, var);
  // return "output_char("+var+",\""+var+"\");\n";
}

InputSpec *CharType::GenerateRandomInput() {
  char c = utils::rand_char();
  std::string spec = "{char: " + std::to_string(c) + "}";
  std::string raw = std::to_string(c);
  return new InputSpec(spec, raw);
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
    ret.push_back(GenerateRandomInput());
  }
  return ret;
}
