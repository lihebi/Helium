#ifndef __TYPE_FACTORY_HPP__
#define __TYPE_FACTORY_HPP__

#include <string>
#include <regex>

#include "type/Type.hpp"

// Why using const will result in warning: unused variable?
// modifier
extern uint8_t CONST_MASK;
extern uint8_t STATIC_MASK;
extern uint8_t EXTERN_MASK;
extern uint8_t VOLATILE_MASK;
// length
extern uint8_t UNSIGNED_MASK;
extern uint8_t SIGNED_MASK;
extern uint8_t SHORT_MASK;
extern uint8_t LONG_MASK;
// primitive
extern uint8_t INT_MASK;
extern uint8_t CHAR_MASK;
extern uint8_t FLOAT_MASK;
extern uint8_t DOUBLE_MASK;
extern uint8_t BOOL_MASK;
// keyword
extern uint8_t STRUCT_MASK;
extern uint8_t ENUM_MASK;

extern std::regex
// modifier
const_regex,
static_regex,
extern_regex,
volatile_regex,
// length
unsigned_regex,
signed_regex,
long_regex,
short_regex,
// primitive
int_regex,
char_regex,
float_regex,
double_regex,
bool_regex,
// keyword
struct_regex,
enum_regex;


class TypeFactory {
public:
  static TypeFactory* Instance() {
    if (m_instance == 0) {
      m_instance = new TypeFactory();
    }
    return m_instance;
  }
  std::shared_ptr<Type> CreateType(const std::string& type_name);
private:
  TypeFactory()
  : m_modifier(0), m_length(0), m_primitive(0), m_keyword(0)
  {}
  ~TypeFactory();

  void clear();
  void decomposite(std::string name);
  bool isPrimitiveType();
  bool isEnumType();
  bool isStructureType();
  bool isSystemType();
  static TypeFactory *m_instance;
  uint8_t m_modifier;
  uint8_t m_length;
  uint8_t m_primitive;
  uint8_t m_keyword;
  int m_dimension;
  int m_pointer_level;
  std::string m_identifier;
};

#endif
