#ifndef __TYPE_H__
#define __TYPE_H__

#include <string>
#include "snippet.h"

enum type_kind {
  ENUM_TYPE,
  PRIM_TYPE,
  STRUCT_TYPE,
  SYSTEM_TYPE,
  UNION_TYPE
};

class Type {
public:
  Type() : m_dimension(0), m_pointer_level(0) {}
  virtual ~Type() {}
  virtual std::string GetInputCode(const std::string& var) const = 0;
  virtual std::string GetInputCodeWithoutDecl(const std::string& var) const = 0;
  virtual std::string GetOutputCode(const std::string& var) const = 0;
  virtual std::string GetInputSpecification() = 0;
  virtual std::string GetOutputSpecification() = 0;
  virtual std::string GetName() const = 0;
  virtual enum type_kind GetTypeKind() const = 0;

  static std::string GetDeclCode(const std::string& type_name, const std::string& var_name, int pointer_level);
  static std::string GetAllocateCode(const std::string& type_name, const std::string& var_name, int pointer_level);
  static std::string GetArrayCode(const std::string& type_name, const std::string& var_name, int dimension);

  void SetDimension(int d);
  int GetDimension() const;
  void SetPointerLevel(int l);
  int GetPointerLevel() const;

private:
  int m_dimension;
  int m_pointer_level;
};


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

class EnumType : public Type {
public:
  EnumType(const std::string& name);
  virtual ~EnumType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetInputCodeWithoutDecl(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
  virtual enum type_kind GetTypeKind() const {return ENUM_TYPE;}
private:
  std::string m_name;

  std::string m_alias;
  // available name for type name. either "struct $m_name" or "$m_alias"
  std::string m_avail_name;
  Snippet* m_snippet;
};

class PrimitiveType : public Type {
public:
  // Deprecated
  PrimitiveType(const struct type_specifier& specifier);
  virtual ~PrimitiveType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetInputCodeWithoutDecl(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const {return m_name;}
  virtual enum type_kind GetTypeKind() const {return PRIM_TYPE;}

private:

  struct type_specifier m_specifier;

  std::string m_name;
};

class StructureType : public Type {
public:
  StructureType(const std::string& name);
  virtual ~StructureType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetInputCodeWithoutDecl(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
  virtual enum type_kind GetTypeKind() const {return STRUCT_TYPE;}
private:
  void simplifyCode();
  void parseFields();

  std::string getPrefix(const std::string& var) const;

  std::vector<std::shared_ptr<Variable> > m_fields;
  std::string m_name;
  // std::string m_alias;
  // // available name for type name. either "struct $m_name" or "$m_alias"
  // std::string m_avail_name;
  Snippet* m_snippet;
  static std::set<std::string> m_recursion_set;
  bool m_null = false;
};

class SystemType : public Type {
public:
  SystemType(const std::string& name, const struct struct_specifier& specifier);
  ~SystemType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetInputCodeWithoutDecl(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
  virtual enum type_kind GetTypeKind() const {return SYSTEM_TYPE;}
private:
  std::string m_name;
  struct struct_specifier m_specifier;
  std::string m_type;
};

class UnionType : public Type {
public:
  UnionType(const std::string& name);
  virtual ~UnionType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetInputCodeWithoutDecl(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual std::string GetInputSpecification();
  virtual std::string GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
  virtual enum type_kind GetTypeKind() const {return UNION_TYPE;}
private:
  std::string m_name;

  std::string m_alias;
  // available name for type name. either "struct $m_name" or "$m_alias"
  std::string m_avail_name;
  Snippet* m_snippet;
};


/*******************************
 ** Variable
 *******************************/

class Variable {
public:
  Variable(std::shared_ptr<Type> type, const std::string& name);
  ~Variable() {}
  const std::string& GetName() const {
    return m_name;
  }
  const std::shared_ptr<Type> GetType() const { return m_type;}
  std::string GetInputCode(const std::string& prefix="") const;
  std::string GetInputCodeWithoutDecl(const std::string& prefix="") const;
  std::string GetOutputCode(const std::string& prefix="") const;
  std::string GetInputSpecification() const;
  std::string GetOutputSpecification() const;

private:
  pugi::xml_node m_node; // the node where the variable is declared
  std::shared_ptr<Type> m_type;
  std::string m_name;
};

class VariableFactory {
public:
  static std::vector<std::shared_ptr<Variable> > FromParamList(pugi::xml_node node);
  static std::vector<std::shared_ptr<Variable> > FromForInit(pugi::xml_node node);
  static std::shared_ptr<Variable> FromDecl(pugi::xml_node node);
  static std::shared_ptr<Variable> FromDeclStmt(pugi::xml_node node);
private:
};

#endif
