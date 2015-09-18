#include "resolver/SystemResolver.hpp"

SystemResolver* SystemResolver::m_instance = 0;

// check whether id can be resolved
// modify m_headers and m_flags
bool SystemResolver::Check(const std::string& id) {
  return false;
}
// resolve to primitive type
std::shared_ptr<Type> ResolveType(const std::string& type);
