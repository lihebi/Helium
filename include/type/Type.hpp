#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include <string>


class Type {
public:
  Type() {}
  virtual ~Type() {}
  virtual std::string GetInputCode(const std::string& var) const = 0;
  virtual std::string GetOutputCode(const std::string& var) const = 0;
  virtual void GetInputSpecification() = 0;
  virtual void GetOutputSpecification() = 0;
private:
};

#endif
