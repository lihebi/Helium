#ifndef VARIABLE_H
#define VARIABLE_H

#include "type.h"

class Variable {
public:
  Variable(Type *type, std::string var) : m_type(type), m_var(var) {}
  Type *GetType() {return m_type;}
  std::string GetName() {return m_var;}
private:
  Type *m_type;
  std::string m_var;
};



#endif /* VARIABLE_H */
