#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include <string>

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

#endif
