#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#include <string>


class Type {
public:
  Type() : m_dimension(0), m_pointer_level(0) {}
  virtual ~Type() {}
  virtual std::string GetInputCode(const std::string& var) const = 0;
  virtual std::string GetOutputCode(const std::string& var) const = 0;
  virtual void GetInputSpecification() = 0;
  virtual void GetOutputSpecification() = 0;
  virtual std::string GetName() const = 0;

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
