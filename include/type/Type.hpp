#ifndef __TYPE_HPP__
#define __TYPE_HPP__

enum type_name {
  INT,
  CHAR,
  FLOAT,
  DOUBLE,
  BOOL
};
enum additional_type {
  UNSIGNED,
  SIGNED,
  SHORT,
  LONG
};
enum type_modifier {
  CONST,
  STATIC,
  EXTERN,
  VOLATILE
};

class Type {
public:
  Type();
  virtual ~Type();
  virtual void GetInputCode() = 0;
  virtual void GetOutputCode() = 0;
  virtual void GetInputSpecification() = 0;
  virtual void GetOutputSpecification() = 0;
private:
};

#endif
