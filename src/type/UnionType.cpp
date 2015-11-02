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
  std::string code;
  if (GetDimension()>0) {
    code = Type::GetArrayCode(m_avail_name.c_str(), var, GetDimension());
  }
  if (GetPointerLevel() > 0) {
    code = Type::GetAllocateCode(m_avail_name, var, GetPointerLevel());
  } else {
    code = m_avail_name + " " + var + ";\n";
  }
  return code;
}

std::string
UnionType::GetInputCodeWithoutDecl(const std::string& var) const {
  return "";
}

std::string UnionType::GetOutputCode(const std::string& var) const {
  return "";
}
std::string
UnionType::GetInputSpecification() {
  return "";
}
std::string
UnionType::GetOutputSpecification() {
  return "";
}
