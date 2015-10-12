#include "type/EnumType.hpp"
#include <set>
#include "resolver/Ctags.hpp"
#include <cassert>

EnumType::EnumType(const std::string& name) : m_name(name) {
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 'g') {
      m_snippet = *it;
      if ((*it)->GetName() == name) {
        m_name = name;
        m_avail_name = "enum " + m_name;
      } else {
        m_alias = name;
        m_avail_name = m_alias;
      }
    }
  }
  assert(!m_avail_name.empty());
}
EnumType::~EnumType() {
}
std::string
EnumType::GetInputCode(const std::string& var) const {
  // TODO parse fields
  // TODO input from outside
  return m_avail_name + " " + var + ";\n";
}
std::string EnumType::GetOutputCode(const std::string& var) const {
  return "";
}
std::string
EnumType::GetInputSpecification() {
  return "";
}
std::string
EnumType::GetOutputSpecification() {
  return "";
}
