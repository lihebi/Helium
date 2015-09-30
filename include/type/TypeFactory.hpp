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
extern uint8_t VOID_MASK;
// keyword
extern uint8_t STRUCT_MASK;
extern uint8_t ENUM_MASK;


class TypeFactory {
public:
  TypeFactory(const std::string& name);
  ~TypeFactory() {}

  std::shared_ptr<Type> CreateType();
  bool IsPrimitiveType();
private:
  void clear();
  void decomposite(std::string name);


  std::string m_name;
  uint8_t m_modifier;
  uint8_t m_length;
  uint8_t m_primitive;
  uint8_t m_keyword;
  int m_dimension;
  int m_pointer_level;
  std::string m_identifier;
};

#endif
