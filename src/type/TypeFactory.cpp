#include "type/TypeFactory.hpp"
#include "type/PrimitiveType.hpp"
#include "type/EnumType.hpp"
#include "type/SystemType.hpp"
#include "type/StructureType.hpp"
#include "util/StringUtil.hpp"

TypeFactory* TypeFactory::m_instance = 0;

bool
TypeFactory::isPrimitiveType() {
  if (m_length || m_primitive) return true;
  else return false;
}
bool
TypeFactory::isEnumType() {
  if (m_keyword & ENUM_MASK) return true;
  else return false;
}
bool
TypeFactory::isSystemType() {
  // TODO system type resolve
  return false;
}
bool
TypeFactory::isStructureType() {
  // TODO structure resolve
  return false;
}

// TODO free the pointer
std::shared_ptr<Type>
TypeFactory::CreateType(const std::string& name) {
  decomposite(name);
  if (isPrimitiveType()) {
    return std::make_shared<PrimitiveType>(m_length, m_primitive);
  } else if (isEnumType()) {
    return std::make_shared<EnumType>(m_identifier);
  } else if (isSystemType()) {
    return std::make_shared<SystemType>(m_identifier);
  } else if (isStructureType()) {
    return std::make_shared<StructureType>(m_identifier);
  } else {
    return NULL;
  }
}

bool search_and_replace(std::string s, std::regex reg) {
  if (std::regex_search(s, reg)) {
    s = std::regex_replace(s, reg, "");
    return true;
  }
  return false;
}

void
TypeFactory::clear() {
  m_modifier = 0;
  m_length = 0;
  m_primitive = 0;
  m_keyword = 0;
  m_identifier = "";
}
void
TypeFactory::decomposite(std::string name) {
  clear();
  if (name.find('[') != -1) {
    // array type should not be here
    name = name.substr(0, name.find('['));
  }
  // modifier
  if (search_and_replace(name, const_regex)) m_modifier |= CONST_MASK;
  if (search_and_replace(name, static_regex)) m_modifier |= STATIC_MASK;
  if (search_and_replace(name, extern_regex)) m_modifier |= EXTERN_MASK;
  if (search_and_replace(name, volatile_regex)) m_modifier |= VOLATILE_MASK;
  // length
  if (search_and_replace(name, unsigned_regex)) m_length |= UNSIGNED_MASK;
  if (search_and_replace(name, signed_regex)) m_length |= SIGNED_MASK;
  if (search_and_replace(name, long_regex)) m_length |= LONG_MASK;
  if (search_and_replace(name, short_regex)) m_length |= SHORT_MASK;
  // primitive
  if (search_and_replace(name, int_regex)) m_primitive |= INT_MASK;
  if (search_and_replace(name, char_regex)) m_primitive |= CHAR_MASK;
  if (search_and_replace(name, float_regex)) m_primitive |= FLOAT_MASK;
  if (search_and_replace(name, double_regex)) m_primitive |= DOUBLE_MASK;
  if (search_and_replace(name, bool_regex)) m_primitive |= BOOL_MASK;
  // other
  if (search_and_replace(name, struct_regex)) m_keyword |= STRUCT_MASK;
  if (search_and_replace(name, enum_regex)) m_keyword |= ENUM_MASK;
  m_identifier = StringUtil::trim(name);
}
