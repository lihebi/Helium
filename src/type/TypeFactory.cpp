#include "type/TypeFactory.hpp"
#include "type/PrimitiveType.hpp"
#include "type/EnumType.hpp"
#include "type/SystemType.hpp"
#include "type/StructureType.hpp"
#include "util/StringUtil.hpp"
#include "resolver/SystemResolver.hpp"
#include <iostream>
#include <cassert>

static std::regex
// modifier
const_regex   ("\\bconst\\b"),
static_regex  ("\\bstatic\\b"),
extern_regex  ("\\bextern\\b"),
volatile_regex("\\bvolatile\\b"),
// length
unsigned_regex("\\bunsigned\\b"),
signed_regex  ("\\bsigned\\b"),
long_regex    ("\\blong\\b"),
short_regex   ("\\bshort\\b"),
// primitive
int_regex     ("\\bint\\b"),
char_regex    ("\\bchar\\b"),
float_regex   ("\\bfloat\\b"),
double_regex  ("\\bdouble\\b"),
bool_regex    ("\\bbool\\b"),
void_regex    ("\\bvoid\\b"),
// keyword
struct_regex  ("\\bstruct\\b"),
enum_regex    ("\\benum\\b");

uint8_t CONST_MASK = 0x01;
uint8_t STATIC_MASK = 0x01 << 1;
uint8_t EXTERN_MASK = 0x01 << 2;
uint8_t VOLATILE_MASK = 0x01 << 3;
// length
uint8_t UNSIGNED_MASK = 0x01;
uint8_t SIGNED_MASK = 0x01 << 1;
uint8_t SHORT_MASK = 0x01 << 2;
uint8_t LONG_MASK = 0x01 << 3;
// primitive
uint8_t INT_MASK = 0x01;
uint8_t CHAR_MASK = 0x01 << 1;
uint8_t FLOAT_MASK = 0x01 << 2;
uint8_t DOUBLE_MASK = 0x01 << 3;
uint8_t BOOL_MASK = 0x01 << 4;
uint8_t VOID_MASK = 0x01 << 5;
// keyword
uint8_t STRUCT_MASK = 0x01;
uint8_t ENUM_MASK = 0x01 << 1;

TypeFactory::TypeFactory(const std::string& name)
: m_name(name), m_modifier(0), m_length(0), m_primitive(0), m_keyword(0) {
  decomposite(m_name);
}

static bool
is_primitive_type(uint8_t length, uint8_t primitive) {
  if (length || primitive) return true;
  else return false;
}

bool
TypeFactory::IsPrimitiveType() {
  return is_primitive_type(m_length, m_primitive);
}

static bool
is_enum_type(uint8_t keyword) {
  if (keyword & ENUM_MASK) return true;
  else return false;
}

static bool
is_system_type(const std::string& identifier) {
  if (SystemResolver::Instance()->Has(identifier)) return true;
  else return false;
}

static bool
is_local_type(const std::string& identifier) {
  if (Ctags::Instance()->Parse(identifier).empty()) return false;
  else return true;
}


std::shared_ptr<Type>
TypeFactory::CreateType() {
  std::shared_ptr<Type> type;
  if (is_primitive_type(m_length, m_primitive)) {
    type = std::make_shared<PrimitiveType>(m_length, m_primitive);
  } else if (is_enum_type(m_keyword)) {
    type = std::make_shared<EnumType>(m_identifier);
  } else if (is_local_type(m_identifier)) {
    // need to know the code for local type
    // only handle structure or typedef
    std::set<Snippet*> snippets = Ctags::Instance()->Resolve(m_identifier);
    for (auto it=snippets.begin();it!=snippets.end();it++) {
      if ((*it)->GetType() == 's') {
        // just create the structure
        type = std::make_shared<StructureType>(m_identifier);
        break;
      } else if ((*it)->GetType() == 't') {
        // extract the to_type, and recursively create type.
        std::string code = (*it)->GetCode();
        code = code.substr(code.find("typedef"));
        code = code.substr(0, code.rfind(';'));
        std::vector<std::string> vs = StringUtil::Split(code);
        // FIXME may not be the middle
        // to_type is the middle part of split
        std::string to_type;
        for (int i=1;i<vs.size()-1;i++) {
          to_type += vs[i]+' ';
        }
        std::string new_name = m_name;
        new_name.replace(new_name.find(m_identifier), m_identifier.length(), to_type);
        // CAUSION this may or may not be a primitive type
        // FIXME may infinite loop?
        type = TypeFactory(new_name).CreateType();
        break;
      }
    }
    // type should contains something.
    // or it may be NULL. So if it fails, do not necessarily means a bug
    assert(type);
  } else if (is_system_type(m_identifier)) {
    // std::cout << m_identifier << std::endl;
    std::string prim_type = SystemResolver::Instance()->ResolveType(m_identifier);
    if (prim_type.empty()) {
      // TODO detail of system type
      type = std::make_shared<SystemType>(m_identifier);
    } else {
      // std::cout << "Resolved from " << m_identifier << " to Primitive: \033[32m" << prim_type << "\033[0m" << std::endl;
      std::string new_name = m_name;
      new_name.replace(new_name.find(m_identifier), m_identifier.length(), prim_type);
      // FIXME this should be a primitive type
      type = TypeFactory(new_name).CreateType();
    }
  } else {
    std::cout << "\033[33m[TypeFactory::CreateType][Warning] Not supported type: "
    << m_identifier << "\033[0m"
    << " in: " << m_name << std::endl;
    return NULL;
  }
  type->SetPointerLevel(m_pointer_level);
  type->SetDimension(m_dimension);
  return type;
}

bool search_and_remove(std::string &s, std::regex reg) {
  if (std::regex_search(s, reg)) {
    s = std::regex_replace(s, reg, "");
    return true;
  }
  return false;
}

int count_and_remove(std::string &s, char c) {
  int count = std::count(s.begin(), s.end(), c);
  if (count) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
  }
  return count;
}

void
TypeFactory::clear() {
  m_modifier = 0;
  m_length = 0;
  m_primitive = 0;
  m_keyword = 0;
  m_identifier = "";
  m_dimension = 0;
  m_pointer_level = 0;
}
void
TypeFactory::decomposite(std::string name) {
  clear();
  if (name.find('[') != -1) {
    m_dimension = std::count(name.begin(), name.end(), '[');
    name = name.substr(0, name.find('['));
  }
  m_pointer_level = count_and_remove(name, '*');
  // modifier
  if (search_and_remove(name, const_regex)) m_modifier |= CONST_MASK;
  if (search_and_remove(name, static_regex)) m_modifier |= STATIC_MASK;
  if (search_and_remove(name, extern_regex)) m_modifier |= EXTERN_MASK;
  if (search_and_remove(name, volatile_regex)) m_modifier |= VOLATILE_MASK;
  // length
  if (search_and_remove(name, unsigned_regex)) m_length |= UNSIGNED_MASK;
  if (search_and_remove(name, signed_regex)) m_length |= SIGNED_MASK;
  if (search_and_remove(name, long_regex)) m_length |= LONG_MASK;
  if (search_and_remove(name, short_regex)) m_length |= SHORT_MASK;
  // primitive
  if (search_and_remove(name, int_regex)) m_primitive |= INT_MASK;
  if (search_and_remove(name, char_regex)) m_primitive |= CHAR_MASK;
  if (search_and_remove(name, float_regex)) m_primitive |= FLOAT_MASK;
  if (search_and_remove(name, double_regex)) m_primitive |= DOUBLE_MASK;
  if (search_and_remove(name, bool_regex)) m_primitive |= BOOL_MASK;
  if (search_and_remove(name, void_regex)) m_primitive |= VOID_MASK;
  // other
  if (search_and_remove(name, struct_regex)) m_keyword |= STRUCT_MASK;
  if (search_and_remove(name, enum_regex)) m_keyword |= ENUM_MASK;

  m_identifier = StringUtil::trim(name);
}
