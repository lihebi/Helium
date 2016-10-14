#ifndef TESTER_H
#define TESTER_H



#include "common.h"
#include "type/type.h"
#include "type/variable.h"


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



class TestSuite {
public:
  TestSuite() {}
  ~TestSuite() {}
  std::string GetInput();
  std::string GetSpec();
  void Add(std::string var, InputSpec *spec) {
    m_data.push_back({var, spec});
  }
private:
  // pair of (varname, InputSpec)
  std::vector<std::pair<std::string, InputSpec*> > m_data;
};

class Tester {
public:
  // Tester(std::string exe_folder, std::string exe, std::map<std::string, Type*> inputs);
  Tester(std::string exe_folder, std::string exe, std::vector<Variable*> inputs);
  ~Tester() {}
  void Test();
  void Analyze(TestResult *result);
  
private:
  void genTestSuite();
  void genRandom();
  void genPairwise();
  void freeTestSuite();

  fs::path m_exe_folder;
  fs::path m_exe;
  // std::map<std::string, Type*> m_inputs;
  std::vector<Variable*> m_inputs;

  std::vector<TestSuite> m_test_suites;

  std::set<InputSpec*> m_specs;
};

#endif /* TESTER_H */
