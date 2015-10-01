#include "type/UnionType.hpp"
#include <set>
#include "resolver/Ctags.hpp"
#include <cassert>

UnionType::UnionType(const std::string& name) : m_name(name) {
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 'u') {
      m_snippet = *it;
      if ((*it)->GetName() == name) {
        m_name = name;
        m_avail_name = "union " + m_name;
      } else {
        m_alias = name;
        m_avail_name = m_alias;
      }
    }
  }
  assert(!m_avail_name.empty());
}
UnionType::~UnionType() {
}
std::string
UnionType::GetInputCode(const std::string& var) const {
  // TODO parse fields
  // TODO input from outside
  return m_avail_name + " " + var + ";\n";
}
std::string UnionType::GetOutputCode(const std::string& var) const {
  return "";
}
void UnionType::GetInputSpecification() {
}
void UnionType::GetOutputSpecification() {
}
