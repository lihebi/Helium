#include "type/PrimitiveType.hpp"
#include "type/TypeFactory.hpp"
#include <iostream>

// Deprecated
PrimitiveType::PrimitiveType(const std::string& name)
: m_name(name) {
  ;
}

PrimitiveType::PrimitiveType(uint8_t length, uint8_t type)
: m_length(length), m_type(type) {
  if (length & UNSIGNED_MASK) m_name += "unsigned ";
  if (length & SIGNED_MASK) m_name += "signed ";
  if (length & SHORT_MASK) m_name += "short ";
  if (length & LONG_MASK) m_name += "long ";
  if (type & INT_MASK) m_name += "int ";
  if (type & CHAR_MASK) m_name += "char ";
  if (type & FLOAT_MASK) m_name += "float ";
  if (type & DOUBLE_MASK) m_name += "double ";
  if (type & BOOL_MASK) m_name += "bool ";
  m_name.pop_back();
  // std::cout << "[PrimitiveType::PrimitiveType] " << m_name << std::endl;
}

PrimitiveType::~PrimitiveType() {}

std::string
PrimitiveType::getIntInputCode(const std::string& var) const {
  if (m_length & UNSIGNED_MASK) {
    // unsigned int
  } else if (m_length & SHORT_MASK) {
    // short
  } else if (m_length & LONG_MASK) {
    // long
  }
  std::string s = "int "+var+";\n";
  s += "scanf(\"%d\", &"+var+");\n";
  return s;
}

std::string
PrimitiveType::GetInputCode(const std::string& var) const {
  std::string s;
  if (m_type) {
    if (m_type & INT_MASK) {
      return getIntInputCode(var);
    } else if (m_type & CHAR_MASK) {
      s += "char "+var+";\n";
      s += "scanf(\"%c\", &"+var+");\n";
    } else if (m_type & FLOAT_MASK) {
      s += "float " + var + ";\n";
      s += "scanf(\"%f\", &"+var+");\n";
    } else if (m_type & DOUBLE_MASK) {
      s += "double " + var + ";\n";
      s += "scanf(\"%lf\", &"+var+");\n";
    } else if (m_type & BOOL_MASK) {
      s += "bool " + var + ";\n";
      s += "scanf(\"%d\", &"+var+");\n";
    }
  } else {
    // int
    return getIntInputCode(var);
  }
  return s;
}

std::string
PrimitiveType::GetOutputCode(const std::string& var) const {
  return "";
}

void
PrimitiveType::GetInputSpecification() {
}

void
PrimitiveType::GetOutputSpecification() {
}
