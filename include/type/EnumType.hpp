#ifndef __ENUM_TYPE_HPP__
#define __ENUM_TYPE_HPP__

#include "type/Type.hpp"
#include "snippet/Snippet.hpp"

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

#endif
