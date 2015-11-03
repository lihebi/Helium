#ifndef __STRUCTURE_TYPE_HPP__
#define __STRUCTURE_TYPE_HPP__

#include "type/Type.hpp"
#include <vector>
#include <memory>
#include "snippet/Snippet.hpp"
#include "variable/Variable.hpp"

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

#endif
