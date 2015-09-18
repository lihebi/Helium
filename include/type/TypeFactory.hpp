#ifndef __TYPE_FACTORY_HPP__
#define __TYPE_FACTORY_HPP__

#include <string>
#include <regex>

#include "type/Type.hpp"

// modifier
const uint8_t CONST_MASK = 0x01;
const uint8_t STATIC_MASK = 0x01 << 1;
const uint8_t EXTERN_MASK = 0x01 << 2;
const uint8_t VOLATILE_MASK = 0x01 << 3;
// length
const uint8_t UNSIGNED_MASK = 0x01;
const uint8_t SIGNED_MASK = 0x01 << 1;
const uint8_t SHORT_MASK = 0x01 << 2;
const uint8_t LONG_MASK = 0x01 << 3;
// primitive
const uint8_t INT_MASK = 0x01;
const uint8_t CHAR_MASK = 0x01 << 1;
const uint8_t FLOAT_MASK = 0x01 << 2;
const uint8_t DOUBLE_MASK = 0x01 << 3;
const uint8_t BOOL_MASK = 0x01 << 4;
// keyword
const uint8_t STRUCT_MASK = 0x01;
const uint8_t ENUM_MASK = 0x01 << 1;


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
  , const_regex("\\bconst\\b"), static_regex("\\bstatic\\b")
  , extern_regex("\\bextern\\b"), volatile_regex("\\bvolatile\\b")
  , unsigned_regex("\\bsigned\\b"), signed_regex("\\bunsigned\\b")
  , long_regex("\\blong\\b"), short_regex("\\bshort\\b")
  , int_regex("\\bint\\b"), char_regex("\\bchar\\b")
  , float_regex("\\bfloat\\b"), double_regex("\\bdouble\\b")
  , bool_regex("\\bbool\\b")
  , struct_regex("\\bstruct\\b"), enum_regex("\\benum\\b")
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
  std::string m_identifier;
  // modifier
  std::regex const_regex;
  std::regex static_regex;
  std::regex extern_regex;
  std::regex volatile_regex;
  // length
  std::regex unsigned_regex;
  std::regex signed_regex;
  std::regex long_regex;
  std::regex short_regex;
  // primitive
  std::regex int_regex;
  std::regex char_regex;
  std::regex float_regex;
  std::regex double_regex;
  std::regex bool_regex;
  // keyword
  std::regex struct_regex;
  std::regex enum_regex;
};

#endif
