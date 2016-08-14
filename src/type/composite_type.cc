#include "type.h"

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

