#ifndef FORMULA_H
#define FORMULA_H

#include "common.h"

typedef enum _FormulaKind {
  FK_Equal,
  FK_NonEqual,
  FK_Larger,
  FK_Less,
  FK_LargerEqual,
  FK_LessEqual
} FormulaKind;

typedef enum _OpeartorKind {
  OK_Plus,
  OK_Minus
} OperatorKind;

/**
 * The formula for a invariants
 * This must be a three address code
 * Or a two address code
 * a = b
 * a = b + c
 * a < b + c
 * Can also accept a+b = c TODO
 */
class Formula {
public:
  static bool is_constant(std::string);
  Formula(std::string lhs, std::string op, std::string rhs);
  virtual ~Formula() {};
  FormulaKind GetKind() {
    return m_kind;
  }
  std::string GetLHS() {
    return m_lhs;
  }
  std::vector<std::string> GetRHS() {
    return m_rhs;
  }
  OperatorKind GetOp() {
    return m_op;
  }
  void Replace(std::map<std::string, std::string> m);
  bool Valid();

  /**
   * Data Validation
   */
  void ClearData() {
    m_lhs_data.clear();
    m_rhs_data.clear();
  }
  void SetLHSData(std::vector<std::string> data) {
    m_lhs_data = data;
  }
  void AddRHSData(std::vector<std::string> data) {
    m_rhs_data.push_back(data);
  }
  virtual bool Validate() = 0;
  virtual bool ValidateVariance() = 0;
  virtual std::string ToString() = 0;
protected:
  FormulaKind m_kind;
  OperatorKind m_op;
  std::string m_lhs;
  std::vector<std::string> m_rhs;

  // data
  std::vector<std::string> m_lhs_data;
  std::vector<std::vector<std::string> > m_rhs_data;
};

class UnaryFormula : public Formula {
public:
  UnaryFormula(std::string lhs, std::string op, std::string rhs) : Formula(lhs, op, rhs) {}
  virtual bool Validate();
  virtual bool ValidateVariance();
  virtual std::string ToString();
private:
  bool validate(int idx);
};

/**
 * TODO refactor BiFormula and tester.cc/BinaryFormula
 */
class BiFormula : public Formula {
public:
  BiFormula(std::string lhs, std::string op, std::string rhs) : Formula(lhs, op, rhs) {}
  virtual bool Validate();
  virtual bool ValidateVariance();
  virtual std::string ToString();
private:
  bool validate(int idx);
};

class FormulaFactory {
public:
  static Formula* CreateFormula(std::string str);
};


#endif /* FORMULA_H */
