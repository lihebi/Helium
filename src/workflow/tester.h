#ifndef TESTER_H
#define TESTER_H

#include "segment.h"


std::map<std::string, std::string> get_header_value_map(std::string output);


/**
 * The test result should record both input and output.
 * Input is used to construct it.
 * Output is added later. You can add arbitrary number of output.
 * This class is able to generate the CSV file for analyze.
 */
class TestResult {
public:
  TestResult(std::vector<std::vector<InputSpec*> > test_suite) : m_test_suite(test_suite) {}
  void GetInvariants();
  void GetPreconditions();
  void GetTransferFunctions();
  void AddOutput(std::string output, bool success) {
    if (success) {
      m_poi_output_success.push_back(output);
    } else {
      m_poi_output_failure.push_back(output);
    }
    m_poi_output.push_back({output, success});
  }
  void PrepareData();
  // DEPRECATED
  // std::string GenerateCSV(std::string io_type, std::string sf_type);

  std::string GenerateCSV();
  
  typedef enum _CSV_SF_Kind {
    CSK_S,
    CSK_F,
    CSK_SF
  } CVS_SF_Kind;
  typedef enum _CSV_IO_Kind {
    CIK_I,
    CIK_O,
    CIK_IO
  } CSV_IO_Kind;
private:
  std::vector<std::string> m_poi_output_success;
  std::vector<std::string> m_poi_output_failure;
  std::vector<std::pair<std::string, bool> > m_poi_output; // the output, and whether the test succeeds
  std::vector<std::vector<InputSpec*> > m_test_suite;
  // this is a pretty good data structure to hold CSV data
  // from the CSV header to its data values
  // The header contains both the poi output, and the preconditions
  // But I need to seperate them
  //
  // How to seperate them?
  // Add Prefix I_ for precondition items
  // Add Prefix O_ for poi output items
  // std::vector<std::map<std::string, std::string> > header_value_maps;

  // The Method PrepareData fills this structrure
  std::vector<std::map<std::string, std::string> > m_header_value_maps;
  std::set<std::string> m_headers;
  std::set<std::string> m_i_headers;
  std::set<std::string> m_o_headers;
};


class BinaryFormula {
public:
  BinaryFormula(std::string raw);
  ~BinaryFormula() {}
  std::string GetLHS() {return m_lhs;}
  std::string GetRHS() {return m_rhs;}
  int GetConf() {return m_conf;}
  std::string GetOP() {return m_op;}
  // FIXME this should not be used actually, because I may modify the binary formula instance
  // std::string GetRaw() {return m_raw;}
  std::string dump() {
    return m_lhs + m_op + m_rhs + " conf: " + std::to_string(m_conf);
  }
  std::set<std::string> GetVars();

  std::string GetLHSVar() {
    return getVar(m_lhs);
  }
  std::string GetRHSVar() {
    return getVar(m_rhs);
  }

  bool IsLeftVar() {
    return is_var(m_lhs);
  }
  bool IsRightVar() {
    return is_var(m_rhs);
  }

  void UpdateRHS(std::string rhs) {m_rhs = rhs;}
  void UpdateLHS(std::string lhs) {m_lhs = lhs;}
  void Update(std::string from, std::string to) {
    if (m_rhs == from) {
      m_rhs = to;
    }
    if (m_lhs == from) {
      m_lhs = to;
    }
  }

  void Inverse();

  std::string ToString() {
    return m_lhs + m_op + m_rhs;
  }
  static bool is_var(std::string v) {
    if (!v.empty() && (v[0] == 'O' || v[0] == 'I')) return true;
    return false;
  }
private:
  std::string getVar(std::string s);

  std::string m_raw;
  std::string m_lhs;
  std::string m_rhs;
  int m_conf = 0;
  std::string m_op;
};

BinaryFormula *merge(BinaryFormula *fm1, BinaryFormula *fm2);

BinaryFormula* derive_key_inv(std::vector<BinaryFormula*> pres, std::vector<BinaryFormula*> trans, BinaryFormula *inv);
BinaryFormula* get_key_inv(std::vector<BinaryFormula*> invs);

#endif /* TESTER_H */
