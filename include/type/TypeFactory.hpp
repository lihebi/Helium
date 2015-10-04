#ifndef __TYPE_FACTORY_HPP__
#define __TYPE_FACTORY_HPP__

#include <string>
#include <regex>

#include "type/Type.hpp"

struct storage_specifier {
  unsigned int is_auto     : 1;
  unsigned int is_register : 1;
  unsigned int is_static   : 1;
  unsigned int is_extern   : 1;
  unsigned int is_typedef  : 1; // not used
};
struct type_specifier {
  unsigned int is_void     : 1;
  unsigned int is_char     : 1;
  unsigned int is_short    : 1;
  unsigned int is_int      : 1;
  unsigned int is_long     : 1; // do not support long long
  unsigned int is_float    : 1;
  unsigned int is_double   : 1;
  unsigned int is_signed   : 1;
  unsigned int is_unsigned : 1;
  unsigned int is_bool     : 1; // not in C standard
};
struct type_qualifier {
  unsigned int is_const    : 1;
  unsigned int is_volatile : 1;
};
struct struct_specifier {
  unsigned int is_struct   : 1;
  unsigned int is_union    : 1;
  unsigned int is_enum     : 1;
};

struct type_component {
  struct storage_specifier storage_specifier;
  struct type_specifier type_specifier;
  struct type_qualifier type_qualifier;
  struct struct_specifier struct_specifier;
};


class TypeFactory {
public:
  TypeFactory(const std::string& name);
  ~TypeFactory() {}

  std::shared_ptr<Type> CreateType();
  bool IsPrimitiveType();
private:
  std::shared_ptr<Type> createLocalType();
  std::shared_ptr<Type> createSystemType();
  void decomposite(std::string name);


  std::string m_name;
  struct type_component m_component;
  int m_dimension;
  int m_pointer_level;
  std::string m_identifier;
};

#endif
