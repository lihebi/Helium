#ifndef ANALYZER_H
#define ANALYZER_H

/**
 * This is going to be the stand alone analyzer
 * The intend is to replace the python and R script.
 * Of course it can not have a strong library support, such as in R
 * But it has advantages:
 * - easy to maintain a data structure hierarchy to store the result
 * - easy to maintain different analyze method, e.g. relationship, transfer function (by maybe only equality)
 * - Don't need to consider the complex coefficients, e.g. 1.00005, by R linear regression
 * - easy to write, C code, easy to integrate with the framework
 *
 * The downside:
 * - hand written, may be slow
 * - no strong library support, e.g. for linear regression. But that is not needed for now btw.
 */

#include "common.h"
#include "formula.h"

/**
 * Data of CSV file.
 */
class CSVData {
};

typedef enum _RelKind {
  RK_Equal,
  RK_Less,
  RK_Larger,
  RK_LessEqual,
  RK_LargerEqual,
  RK_NA
} RelKind;

class TestSummary {
public:
  // some statistics about the CSV file
  int total_test = 0;
  int reach_poi_test_success = 0;
  int reach_poi_test_failure = 0;
  int no_reach_poi_test_success = 0;
  int no_reach_poi_test_failure = 0;
};



/**
 * Takes CSV files as input.
 *
 * Format of CSV file:
 *
 * I_xxx: input variables
 * O_xxx: output variable at POI
 * HELIUM_TEST_SUCCESS: whether test success or not
 *
 * The data can be:
 * - a number
 * - NULL !NULL
 * - true
 * - NA
 */
class Analyzer {
public:
  Analyzer(std::string csv_file, std::set<std::string> conditions);
  ~Analyzer();
  std::vector<std::string> GetInvariants();
  std::vector<std::string> GetTransferFunctions();
  std::vector<std::string> GetPreConditions();

  TestSummary GetSummary() {return m_summary;}
private:
  // relationship checking
  // > < = <= >=
  std::string checkRelation(std::string h1, std::string h2);
  std::string checkConstant(std::string header);
  std::string checkTransfer(std::string h1, std::string h2);
  std::string checkTemplate(Formula *f);

  std::vector<std::string> checkDiscoveredConstants(std::string header);
  // conditions
  void processCSVFile(std::string csv_file);
  void processConditions(std::set<std::string> conditions);
  void createSimplifiedHeader();

  TestSummary m_summary;
  
  // data
  std::vector<std::string> m_header; // first row in the csv file
  std::map<std::string, std::string> m_simplified_header_m;
  std::vector<std::vector<std::string> > m_raw_data; // this is rows of data in csv
  int m_data_dim = 0; // dimension of valid data, a.k.a. how many valid tests
  // processed data
  std::vector<std::string> m_i_header;
  std::vector<std::string> m_o_header;
  std::map<std::string, std::vector<std::string> > m_data_m; // from header to its data, this is column based
  // result
  // discovered constant
  std::set<int> m_discovered_constants;

  // std::set<std::string> m_conditions;
  std::set<Formula*> m_templates;
};


#endif /* ANALYZER_H */
