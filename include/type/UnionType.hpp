#ifndef __UNION_TYPE_HPP__
#define __UNION_TYPE_HPP__

#include "type/Type.hpp"
#include "snippet/Snippet.hpp"

class UnionType : public Type {
public:
  UnionType(const std::string& name);
  virtual ~UnionType();
  virtual std::string GetInputCode(const std::string& var) const;
  virtual std::string GetOutputCode(const std::string& var) const;
  virtual void GetInputSpecification();
  virtual void GetOutputSpecification();
  virtual std::string GetName() const { return m_name;}
private:
  std::string m_name;

  std::string m_alias;
  // available name for type name. either "struct $m_name" or "$m_alias"
  std::string m_avail_name;
  Snippet* m_snippet;
};

#endif
