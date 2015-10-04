#ifndef __STRUCTURE_TYPE_HPP__
#define __STRUCTURE_TYPE_HPP__

#include "type/Type.hpp"
#include <vector>
#include "snippet/Snippet.hpp"

class StructureType : public Type {
public:
  StructureType(const std::string& name);
  virtual ~StructureType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
private:
  void simplifyCode();
  void parseFields();

  // (Type1, field1), (Type2, field2), ...
  std::vector<std::pair<std::shared_ptr<Type>, std::string> > m_fields;
  std::string m_name;
  // std::string m_alias;
  // // available name for type name. either "struct $m_name" or "$m_alias"
  // std::string m_avail_name;
  Snippet* m_snippet;
};

#endif
