#ifndef VARIABLE_H
#define VARIABLE_H

#include "type.h"

class Variable {
public:
  Variable(Type *type, std::string var) : m_type(type), m_var(var) {}
  Type *GetType() {return m_type;}
  std::string GetName() {return m_var;}
  void SetGlobal(bool b=true) {m_is_global=b;}
  std::string GetDeclCode();
  std::string GetInputCode();
  std::string GetOutputCode();
private:
  Type *m_type;
  std::string m_var;
  bool m_is_global=false;
};



#endif /* VARIABLE_H */
