#include "type/TypeFactory.hpp"
#include "type/PrimitiveType.hpp"
#include "type/EnumType.hpp"
#include "type/SystemType.hpp"
#include "type/StructureType.hpp"
#include "type/UnionType.hpp"
#include "util/StringUtil.hpp"
#include "resolver/SystemResolver.hpp"
#include <iostream>
#include <cassert>
#include "snippet/TypedefSnippet.hpp"

static bool
search_and_remove(std::string &s, std::regex reg) {
  if (std::regex_search(s, reg)) {
    s = std::regex_replace(s, reg, "");
    return true;
  }
  return false;
}

static int
count_and_remove(std::string &s, char c) {
  int count = std::count(s.begin(), s.end(), c);
  if (count) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
  }
  return count;
}

static void
fill_storage_specifier(std::string& name, struct storage_specifier& specifier) {
  specifier.is_auto     = search_and_remove(name, std::regex("\\bauto\\b"))     ? 1 : 0;
  specifier.is_register = search_and_remove(name, std::regex("\\bregister\\b")) ? 1 : 0;
  specifier.is_static   = search_and_remove(name, std::regex("\\bstatic\\b"))   ? 1 : 0;
  specifier.is_extern   = search_and_remove(name, std::regex("\\bextern\\b"))   ? 1 : 0;
  // specifier.is_typedef     = search_and_remove(name, typedef_regex)     ? 1 : 0;
}

static void
fill_type_specifier(std::string& name, struct type_specifier& specifier) {
  specifier.is_void     = search_and_remove(name, std::regex("\\bvoid\\b"))     ? 1 : 0;
  specifier.is_char     = search_and_remove(name, std::regex("\\bchar\\b"))     ? 1 : 0;
  specifier.is_short    = search_and_remove(name, std::regex("\\bshort\\b"))    ? 1 : 0;
  specifier.is_int      = search_and_remove(name, std::regex("\\bint\\b"))      ? 1 : 0;
  specifier.is_long     = search_and_remove(name, std::regex("\\blong\\b"))     ? 1 : 0;
  specifier.is_float    = search_and_remove(name, std::regex("\\bfloat\\b"))    ? 1 : 0;
  specifier.is_double   = search_and_remove(name, std::regex("\\bdouble\\b"))   ? 1 : 0;
  specifier.is_signed   = search_and_remove(name, std::regex("\\bsigned\\b"))   ? 1 : 0;
  specifier.is_unsigned = search_and_remove(name, std::regex("\\bunsigned\\b")) ? 1 : 0;
  specifier.is_bool     = search_and_remove(name, std::regex("\\bbool\\b"))     ? 1 : 0;
}
static void
fill_type_qualifier(std::string& name, struct type_qualifier& qualifier) {
  qualifier.is_const    = search_and_remove(name, std::regex("\\bconst\\b"))    ? 1 : 0;
  qualifier.is_volatile = search_and_remove(name, std::regex("\\bvolatile\\b")) ? 1 : 0;
}

static void
fill_struct_specifier(std::string& name, struct struct_specifier& specifier) {
  specifier.is_struct = search_and_remove(name, std::regex("\\bstruct\\b")) ? 1 : 0;
  specifier.is_enum   = search_and_remove(name, std::regex("\\benum\\b"))   ? 1 : 0;
  specifier.is_union  = search_and_remove(name, std::regex("\\bunion\\b"))  ? 1 : 0;
}

TypeFactory::TypeFactory(const std::string& name)
: m_name(name), m_dimension(0), m_pointer_level(0) {
  std::string name_tmp = m_name;
  if (name_tmp.find('[') != -1) {
    m_dimension = std::count(name_tmp.begin(), name_tmp.end(), '[');
    name_tmp = name_tmp.substr(0, name_tmp.find('['));
  }
  m_pointer_level = count_and_remove(name_tmp, '*');
  fill_storage_specifier(name_tmp, m_component.storage_specifier);
  fill_type_specifier(name_tmp, m_component.type_specifier);
  fill_type_qualifier(name_tmp, m_component.type_qualifier);
  fill_struct_specifier(name_tmp, m_component.struct_specifier);
  m_identifier = StringUtil::trim(name_tmp);
}

bool
TypeFactory::IsPrimitiveType() {
  return m_identifier.empty();
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
TypeFactory::createLocalType() {
  std::shared_ptr<Type> type;
  // need to know the code for local type
  // only handle structure or typedef
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(m_identifier);
  // scan for the first pass, for 's' or 'g' snippets
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 's') {
      // just create the structure
      type = std::make_shared<StructureType>(m_identifier);
      break;
    } else if ((*it)->GetType() == 'g') {
      // this is the typedef enum
      type = std::make_shared<EnumType>(m_identifier);
    } else if ((*it)->GetType() == 'u') {
      type = std::make_shared<UnionType>(m_identifier);
    }
  }
  if (!type) {
    // separate 't' with 'gs' to fix the bug: typedef struct conn conn
    // resolve conn as the true code struct conn {} first possible, so that no recursive
    for (auto it=snippets.begin();it!=snippets.end();it++) {
      if ((*it)->GetType() == 't') {
        if (((TypedefSnippet*)*it)->GetTypedefType() == TYPEDEF_TYPE) {
          std::string to_type = ((TypedefSnippet*)*it)->GetToType();
          std::string new_name = m_name;
          new_name.replace(new_name.find(m_identifier), m_identifier.length(), to_type);
          // CAUSION this may or may not be a primitive type
          // FIXME may infinite loop?
          // YES, by typedef struct conn conn
          type = TypeFactory(new_name).CreateType();
        } else if (((TypedefSnippet*)*it)->GetTypedefType() == TYPEDEF_FUNC_POINTER) {
          std::cout << "[TypeFactory::CreateType]"
          << "\033[33m" << "typedef" + m_identifier + "is function pointer" << "\033[0m"
          << std::endl;
        }
        break;
      }
    }
  }
  // type should contains something.
  // or it may be NULL. So if it fails, do not necessarily means a bug
  if (!type) {
    std::cout << "[TypeFactory::CreateType]"
    << "\033[31m"
    << "the type is local, but is not s or t: " << m_identifier
    << "\033[0m" << std::endl;
    // getchar();
    return NULL;
  }
  return type;
}

std::shared_ptr<Type>
TypeFactory::createSystemType() {
  std::shared_ptr<Type> type;
  std::string prim_type = SystemResolver::Instance()->ResolveType(m_identifier);
  if (prim_type.empty()) {
    // TODO detail of system type
    type = std::make_shared<SystemType>(m_identifier, m_component.struct_specifier);
  } else {
    // std::cout << "Resolved from " << m_identifier << " to Primitive: \033[32m" << prim_type << "\033[0m" << std::endl;
    std::string new_name = m_name;
    new_name.replace(new_name.find(m_identifier), m_identifier.length(), prim_type);
    // FIXME this should be a primitive type
    type = TypeFactory(new_name).CreateType();
  }
  return type;
}

std::shared_ptr<Type>
TypeFactory::CreateType() {
  // std::cout << "[TypeFactory::CreateType]" << m_name << std::endl;
  std::shared_ptr<Type> type;
  if (IsPrimitiveType()) {
    type = std::make_shared<PrimitiveType>(m_component.type_specifier);
  } else if (is_local_type(m_identifier)) {
    type = createLocalType();
  } else if (is_system_type(m_identifier)) {
    type = createSystemType();
  } else {
    std::cout << "\033[33m[TypeFactory::CreateType][Warning] Not supported type: "
    << m_identifier << "\033[0m"
    << " in: " << m_name << std::endl;
    return NULL;
  }
  if (type) {
    type->SetPointerLevel(m_pointer_level);
    type->SetDimension(m_dimension);
  }
  return type;
}
