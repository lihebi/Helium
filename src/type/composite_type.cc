#include "type.h"

StructType::StructType(int snippet_id) : m_snippet_id(snippet_id) {
}
std::string StructType::GetDeclCode(std::string var) {
  // Decl code
  return "";
}
std::string StructType::GetInputCode(std::string var) {
  // the input code should initiate all the fields
  return "";
}
std::string StructType::GetOutputCode(std::string var) {
  // output each field
  return "";
}
InputSpec* StructType::GenerateRandomInput() {
  return NULL;
}
std::string StructType::GetRaw() {
  return "";
}


std::string StructInputSpec::GetRaw() {
  std::string ret;
  std::vector<std::string> list;
  for (auto m : m_fields) {
    InputSpec *spec = m.second;
    if (spec) {
      list.push_back(spec->GetRaw());
    }
  }
  std::string joined = boost::algorithm::join(list, " ");
  ret = joined;
  return ret;
}

std::string StructInputSpec::GetSpec() {
  std::string ret;
  ret += "{";
  std::vector<std::string> list;
  for (auto m : m_fields) {
    std::string name = m.first;
    InputSpec *spec = m.second;
    if (spec) {
      list.push_back(name + ": " + spec->GetSpec());
    }
  }
  std::string joined = boost::algorithm::join(list, ", ");
  ret += joined;
  ret += "}";
  return ret;
}

