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

  void SetDimension(int d) {m_dimension = d;}
  void SetPointerLevel(int l) {m_pointer_level = l;}
private:
  void simplifyCode();
  void parseFields();

  // (Type1, field1), (Type2, field2), ...
  std::vector<std::pair<std::shared_ptr<Type>, std::string> > m_fields;

  int m_dimension;
  int m_pointer_level;
  std::string m_name;
  std::string m_alias;
  Snippet* m_snippet;
};

#endif
