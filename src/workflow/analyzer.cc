#include "analyzer.h"
#include "utils/utils.h"

#include "gtest/gtest.h"
#include "utils/log.h"
#include "helium_options.h"

/********************************
 * Helper functions
 *******************************/

std::string rel_to_string(std::string h1, std::string h2, RelKind k) {
  switch (k) {
  case RK_Equal:
    return h1 + "=" + h2;
  case RK_Less:
    return h1 + "<" + h2;
  case RK_LessEqual:
    return h1 + "<=" + h2;
  case RK_Larger:
    return h1 + ">" + h2;
  case RK_LargerEqual:
    return h1 + ">=" + h2;
  case RK_NA:
    assert(false);
  default: assert(false);
  }
  assert(false);
}


/**
 * Remove NA rows
 */
static void validate(std::vector<std::string> &d1, std::vector<std::string> &d2) {
  assert(d1.size() == d2.size());
  int size = d1.size();
  std::vector<std::string> ret1,ret2;
  for (int i=0;i<size;i++) {
    if (d1[i] == "NA" || d2[i] == "NA") continue;
    ret1.push_back(d1[i]);
    ret2.push_back(d2[i]);
  }
  d1 = ret1;
  d2 = ret2;
}

static void validate(std::vector<std::string> &d) {
  std::vector<std::string> ret;
  for (std::string s : d) {
    if (s != "NA") ret.push_back(s);
  }
  d = ret;
}

static bool validate_number(std::vector<std::string> &d) {
  for (std::string s : d) {
    if (!utils::is_number(s)) {
      std::cout << s  << "\n";
      return false;
    }
  }
  return true;
}

static bool validate_addr(std::vector<std::string> &d) {
  for (std::string s : d) {
    if (s.size() < 2 || s[0] != '0' || s[1] != 'x') return false;
  }
  return true;
}

/**
 * Check if the distance of d2-d1 is constant
 * the result is d2 = d1 + b, or empty
 */
std::string check_const_dist(std::vector<std::string> d1, std::vector<std::string> d2) {
  int size = d1.size();
  // should be numbers to compare
  if (!validate_number(d1)) return "";
  if (!validate_number(d2)) return "";
  // check
  // assert(size > 0);
  if (size == 0) return "";
  int i1 = atoi(d1[0].c_str());
  int i2 = atoi(d2[0].c_str());
  int dist = i2 - i1;
  for (int i=0;i<size;i++) {
    int i1 = atoi(d1[i].c_str());
    int i2 = atoi(d2[i].c_str());
    if (i2 - i1 != dist) return "";
  }
  return std::to_string(dist);
}

/**
 * Integer
 */
std::string check_const_dist_d(std::vector<std::string> d1, std::vector<std::string> d2) {
  int size = d1.size();
  // should be numbers to compare. This should be assert
  // if (!validate_number(d1)) return "";
  // if (!validate_number(d2)) return "";
  assert(validate_number(d1));
  assert(validate_number(d2));
  // check
  // assert(size > 0);
  if (size == 0) return "";
  int i1 = atoi(d1[0].c_str());
  int i2 = atoi(d2[0].c_str());
  int dist = i2 - i1;
  for (int i=0;i<size;i++) {
    int i1 = atoi(d1[i].c_str());
    int i2 = atoi(d2[i].c_str());
    if (i2 - i1 != dist) return "";
  }
  return std::to_string(dist);
}

/**
 * Hex Address
 */
std::string check_const_dist_p(std::vector<std::string> d1, std::vector<std::string> d2) {
  int size = d1.size();
  // should be numbers to compare
  // if (!validate_number(d1)) return "";
  // if (!validate_number(d2)) return "";
  // assert address
  assert(validate_addr(d1));
  assert(validate_addr(d2));
  // check
  // assert(size > 0);
  if (size == 0) return "";
  long int p1 = strtol(d1[0].c_str(), NULL, 16);
  long int p2 = strtol(d2[0].c_str(), NULL, 16);
  // int i1 = atoi(d1[0].c_str());
  // int i2 = atoi(d2[0].c_str());
  // int dist = i2 - i1;
  long int dist = p2 - p1;
  for (int i=0;i<size;i++) {
    long int p1 = strtol(d1[i].c_str(), NULL, 16);
    long int p2 = strtol(d2[i].c_str(), NULL, 16);
    // TODO the distance is larger than something is also useful
    if (p2 - p1 != dist) return "";
  }
  return std::to_string(dist);
  // for (int i=0;i<size;i++) {
  //   int i1 = atoi(d1[i].c_str());
  //   int i2 = atoi(d2[i].c_str());
  //   if (i2 - i1 != dist) return "";
  // }
  // return std::to_string(dist);
}



std::string check_constant(std::vector<std::string> d) {
  int size = (int)d.size();
  std::string last;
  for (int idx=0;idx<size;idx++) {
    // if (d[idx] == "NA") continue;
    if (last.empty()) {
      last = d[idx];
    } else {
      if (last != d[idx]) return "";
    }
  }
  // no valid row detected
  if (last.empty()) return "";
  return last;
}

/**
 * DEPRECATED use the _d and _p version instead, to handle specific types
 */
RelKind check_relation(std::vector<std::string> d1, std::vector<std::string> d2) {
  validate(d1, d2);
  assert(d1.size() == d2.size());
  int size = (int)d1.size();
  // assert(size > 0); // should have at least one data row
  if (size == 0) return RK_NA;
  // should be numbers to compare
  if (!validate_number(d1)) return RK_NA;
  if (!validate_number(d2)) return RK_NA;
  // compuare these at the same time
  bool is_equal = true;
  bool is_less = true;
  bool is_less_equal = true;
  bool is_larger = true;
  bool is_larger_equal = true;
  for (int idx=0;idx<size;idx++) {
    // if any one is not available, just discard this record
    // FIXME but what is all NAs?
    // if (d1[idx] == "NA" || d2[idx] == "NA") continue;
    int i1 = atoi(d1[idx].c_str());
    int i2 = atoi(d2[idx].c_str());
    if (i1 != i2) {
      is_equal = false;
      is_larger = false;
      is_less = false;
    }
    if (i1 < i2) {
      is_larger = false;
      is_larger_equal = false;
    }
    if (i1 > i2) {
      is_less = false;
      is_less_equal = false;
    }
  }
  // return the result in order
  if (is_equal) return RK_Equal;
  else if (is_less) return RK_Less;
  else if (is_larger) return RK_Larger;
  else if (is_less_equal) return RK_LessEqual;
  else if (is_larger_equal) return RK_LargerEqual;
  else return RK_NA;
}

RelKind check_relation_d(std::vector<std::string> d1, std::vector<std::string> d2) {
  validate(d1, d2);
  assert(d1.size() == d2.size());
  int size = (int)d1.size();
  // assert(size > 0); // should have at least one data row
  if (size == 0) return RK_NA;
  // should be numbers to compare
  // if (!validate_number(d1)) return RK_NA;
  // if (!validate_number(d2)) return RK_NA;
  assert(validate_number(d1));
  assert(validate_number(d2));
  // compuare these at the same time
  bool is_equal = true;
  bool is_less = true;
  bool is_less_equal = true;
  bool is_larger = true;
  bool is_larger_equal = true;
  for (int idx=0;idx<size;idx++) {
    int i1 = atoi(d1[idx].c_str());
    int i2 = atoi(d2[idx].c_str());
    if (i1 != i2) {
      is_equal = false;
      is_larger = false;
      is_less = false;
    }
    if (i1 < i2) {
      is_larger = false;
      is_larger_equal = false;
    }
    if (i1 > i2) {
      is_less = false;
      is_less_equal = false;
    }
  }
  // return the result in order
  if (is_equal) return RK_Equal;
  else if (is_less) return RK_Less;
  else if (is_larger) return RK_Larger;
  else if (is_less_equal) return RK_LessEqual;
  else if (is_larger_equal) return RK_LargerEqual;
  else return RK_NA;
}

/**
 * Check Relationship between two pointer addresses
 */
RelKind check_relation_p(std::vector<std::string> d1, std::vector<std::string> d2) {
  validate(d1, d2);
  assert(d1.size() == d2.size());
  int size = (int)d1.size();
  // assert(size > 0); // should have at least one data row
  if (size == 0) return RK_NA;
  // should be numbers to compare
  // if (!validate_number(d1)) return RK_NA;
  // if (!validate_number(d2)) return RK_NA;
  // assert(validate_number(d1));
  // assert(validate_number(d2));
  // compuare these at the same time
  bool is_equal = true;
  bool is_less = true;
  bool is_less_equal = true;
  bool is_larger = true;
  bool is_larger_equal = true;
  for (int idx=0;idx<size;idx++) {
    // int i1 = atoi(d1[idx].c_str());
    // int i2 = atoi(d2[idx].c_str());
    long int p1 = strtol(d1[idx].c_str(), NULL, 16);
    long int p2 = strtol(d2[idx].c_str(), NULL, 16);
    if (p1 != p2) {
      is_equal = false;
      is_larger = false;
      is_less = false;
    }
    if (p1 < p2) {
      is_larger = false;
      is_larger_equal = false;
    }
    if (p1 > p2) {
      is_less = false;
      is_less_equal = false;
    }
  }
  // return the result in order
  if (is_equal) return RK_Equal;
  else if (is_less) return RK_Less;
  else if (is_larger) return RK_Larger;
  else if (is_less_equal) return RK_LessEqual;
  else if (is_larger_equal) return RK_LargerEqual;
  else return RK_NA;
}


/********************************
 * The Analyzer Class
 *******************************/

/**
 * 1. identify valid csv, and output summary information
 * 2. write to file
 * 3. construct m_header
 * 4. construct data into m_raw_data
 */
void Analyzer::processCSVFile(std::string csv_file) {
  helium_print_trace("Analyzer::processCSVFile");
  std::ifstream is;
  is.open(csv_file);
  assert(is.is_open());
  // I'm going to collect some summary metrics for the CSV file
  int total = 0;
  int reach_poi_test_success = 0;
  int reach_poi_test_failure = 0;
  int no_reach_poi_test_success = 0;
  int no_reach_poi_test_failure = 0;
  // valid_csv is for output purpose
  // the output file name is csv_file + "-valid.csv"
  std::string valid_csv;
  std::string valid_csv_file = csv_file + "-valid.csv";
  if (is.is_open()) {
    std::string line;
    getline(is, line);
    valid_csv += line + "\n";
    // constructing m_headers
    // this is a vector, meaning the duplication is not removed
    // so it should be desired to make sure no duplication envolved in the output csv file
    m_header = utils::split(line, ',');

    int poi_idx = -1;
    int poi_out_idx = -1;
    int test_success_idx = -1;
    for (int idx = 0;idx< (int)m_header.size();idx++) {
      if (m_header[idx] == "HELIUM_POI") poi_idx = idx;
      if (m_header[idx] == "HELIUM_POI_OUT_END") poi_out_idx = idx;
      if (m_header[idx] == "HELIUM_TEST_SUCCESS") test_success_idx = idx;
    }
    // assert(m_header.size() >= 3);
    // assert(m_header[0] == "HELIUM_POI");
    // assert(m_header[1] == "HELIUM_POI_OUT_END");
    // assert(m_header[2] == "HELIUM_TEST_SUCCESS");
    while (getline(is, line)) {
      // I need to remove some invalid rows
      // the rows I want is:
      // HELIUM_POI: true
      // TODO HELIUM_POI_OUT_END: true
      // HELIUM_TEST_SUCCESS: false or true
      std::vector<std::string> row = utils::split(line, ',');
      total++;
      if (poi_idx != -1 && row[poi_idx] == "true" // POI
          && poi_out_idx != -1 && row[poi_out_idx] == "true" // POI_OUT
          ) {
        // reach POI
        if (test_success_idx != -1 && row[test_success_idx] == "true") {
          reach_poi_test_success++;
        } else {
          // HERE is the valid row
          valid_csv += line + "\n";
          m_raw_data.push_back(row);
          reach_poi_test_failure++;
        }
      } else {
        // assert(test_success_idx != -1);
        // assert(test_success_idx < (int)row.size());
        if (test_success_idx < (int)row.size()) {
          if (row[test_success_idx] == "true") {
            no_reach_poi_test_success++;
          } else {
            no_reach_poi_test_failure++;
          }
        } else {
          // TODO FIXME
        }
      }
    }
    is.close();
  } else {
    assert(false);
  }

  utils::write_file(valid_csv_file, valid_csv);
  // std::cout << "Valid data: " << m_raw_data.size()  << "\n";
  if (HeliumOptions::Instance()->GetBool("print-csv-summary")) {
    std::cerr << "------ CSV summary ------" << "\n";
    std::cout << "| total records: " << total  << "\n";
    std::cout << "| reach POI, return zero: " << reach_poi_test_success  << "\n";
    std::cout << "| reach POI, return non-zero: " << reach_poi_test_failure  << "\n";
    std::cout << "| no reach POI, return zero: " << no_reach_poi_test_success  << "\n";
    std::cout << "| no reach POI, return non-zero: " << no_reach_poi_test_failure  << "\n";
    std::cout << "-------------------------"  << "\n";
  }

  if (reach_poi_test_success + reach_poi_test_failure >0) {
    helium_dump("reach_poi\n");
  } else {
    helium_dump("no_poi\n");
  }
  std::cout << "Valid tests: " << reach_poi_test_failure  << "\n";

  m_summary.total_test = total;
  m_summary.reach_poi_test_success = reach_poi_test_success;
  m_summary.reach_poi_test_failure = reach_poi_test_failure;
  m_summary.no_reach_poi_test_success = no_reach_poi_test_success;
  m_summary.no_reach_poi_test_failure = no_reach_poi_test_failure;

  // the dimension of data, aka how many valid test
  // this is used to construct a data column when the item to compare is constant
  // FIXME only "valid" data?
  m_data_dim = m_raw_data.size();
  // std::cout << "raw data:"  << "\n";
  // std::cout << m_raw_data.size()  << "\n";
  // for (auto &v : m_raw_data) {
  //   for (auto s : v) {
  //     std::cout << s << " ";
  //   }
  //   std::cout << "\n";
  // }
  // std::cout << "header size: "  << "\n";
  // std::cout << m_header.size()  << "\n";
  
}

/**
 * Special headers:
 * - HELIUM_TEST_SUCCESS: the return code
 * - HELIUM_POI: whether the POI right before POI is printed
 */
Analyzer::Analyzer(std::string csv_file, std::set<std::string> conditions) {
  helium_print_trace("Analyzer::Analyzer");
  processCSVFile(csv_file);
  // process the raw data
  for (int i=0;i<(int)m_header.size();++i) {
    std::string header = m_header[i];
    for (int j=0;j<(int)m_raw_data.size();++j) {
      m_data_m[header].push_back(m_raw_data[j][i]);
    }
  }
  // std::cout << m_data_m.size()  << "\n";
  // construct i header and o header
  for (std::string h : m_header) {
    assert(h.size() > 0);
    if (h[0] == 'I') {
      m_i_header.push_back(h);
    } else if (h[0] == 'O') {
      m_o_header.push_back(h);
    } else {
      // No code here. It might be HELIUM_TEST_SUCCESS
    }
  }

  // OK, I'm going to process the conditions, and possibly associate the variables used in conditons with the m_header
  createSimplifiedHeader();
  processConditions(conditions);
}

Analyzer::~Analyzer() {
  for (Formula *f : m_templates) {
    if (f) delete f;
  }
}

void Analyzer::createSimplifiedHeader() {
  helium_print_trace("Analyzer::createSimplifiedHeader");
  // std::map<std::string, std::string> simplified_header;
  for (std::string h : m_header) {
    if (h.size() > 3) {
      if (h[2] == '_') {
        std::string simp;
        simp = h.substr(3);
        if (simp.find('[') != std::string::npos) {
          // if has surfix, simply discard
          continue;
          simp = simp.substr(0, simp.find('['));
        }
        m_simplified_header_m[simp] = h;
      }
    }
  }
}

/**
 * Process conditions.
 * These conditions are the template for generate invariants
 * 1. filter only the simple ones
 * 1. associate the variable names with the headers found. Discard those that cannot be fully resolved in this way.
 * @return a set of string template to check
 */
void Analyzer::processConditions(std::set<std::string> conditions) {
  helium_print_trace("Analyzer::processConditions");
  // std::set<std::string> ret;
  for (std::string condition : conditions) {
    // FIXME formula needs to be free-d
    Formula *formula = FormulaFactory::CreateFormula(condition);
    if (!formula) {
      continue;
    }
    // replace with header variables
    formula->Replace(m_simplified_header_m);
    if (!formula->Valid()) {
      delete formula;
      continue;
    }
    m_templates.insert(formula);
    // TODO I might want some additional processing, such as:
    // 1. negate the condition
    // 2. increase a constant amount to one side
  }
}



/**
 * OK, now I COMPUTE the invairants, instead of GET
 * 1. constants, e.g. a == NULL
 * 2. relations, e.g. a < b
 */
std::vector<std::string> Analyzer::GetInvariants() {
  helium_print_trace("Analyzer::GetInvariants()");
  std::vector<std::string> ret;
  // invariants cares about the Out variables
  for (int i=0;i<(int)m_o_header.size();++i) {
    // check constant
    std::string res = checkConstant(m_o_header[i]);
    if (!res.empty()) {
      ret.push_back(res);
    }
    // pairwise comparison
    for (int j=i+1;j<(int)m_o_header.size();++j) {
      // compare i and j
      std::string res = checkRelation(m_o_header[i], m_o_header[j]);
      if (!res.empty()) {
        ret.push_back(res);
      }
      // if (kind != RK_NA) {
      //   std::string s = rel_to_string(m_o_header[i], m_o_header[j], kind);
      //   ret.push_back(s);
      // }
    }
  }
  /**
   * For the templates (bulit from the conditions from the loop, currently)
   * Check if it is satisfied
   */
  for (Formula *f : m_templates) {
    std::string res = checkTemplate(f);
    if (!res.empty()) {
      ret.push_back(res);
    }
  }
  return ret;
}

/**
 * Transfer functions capture the relation from Input to Output
 */
std::vector<std::string> Analyzer::GetTransferFunctions() {
  helium_print_trace("Analyzer::GetTransferFunctions()");
  std::vector<std::string> ret;
  for (int i=0;i<(int)m_i_header.size();i++) {
    for (int o=0;o<(int)m_o_header.size();o++) {
      std::string trans = checkTransfer(m_i_header[i], m_o_header[o]);
      if (!trans.empty()) {
        ret.push_back(trans);
      }
    }
  }
  return ret;
}

/**
 * Pre conditions are invariants for Input variables
 */
std::vector<std::string> Analyzer::GetPreConditions() {
  helium_print_trace("Analyzer::GetPreConditions()");
  std::vector<std::string> ret;
  // invariants cares about the Out variables
  for (int i=0;i<(int)m_i_header.size();++i) {
    // check constant
    std::string res = checkConstant(m_i_header[i]);
    if (!res.empty()) {
      ret.push_back(res);
    }
    // for pre-conditions, we need to check for each column, against the discovered constants
    std::vector<std::string> vs = checkDiscoveredConstants(m_i_header[i]);
    ret.insert(ret.end(), vs.begin(), vs.end());
    // not necessary to check the pairwise comparison actually, because the input parameters are huge ...
    // e.g. when the input variable is a char**
    // pairwise comparison
    for (int j=i+1;j<(int)m_i_header.size();++j) {
      // compare i and j
      std::string res = checkRelation(m_i_header[i], m_i_header[j]);
      if (!res.empty()) {
        ret.push_back(res);
      }
      // if (kind != RK_NA) {
      //   std::string s = rel_to_string(m_i_header[i], m_i_header[j], kind);
      //   ret.push_back(s);
      // }
    }
  }
  return ret;
}

/********************************
 * Private Methods
 *******************************/


/**
 * Return the ready formula, not just the constant
 */
std::string Analyzer::checkConstant(std::string header) {
  // std::cout << "checking constant: " << header  << "\n";
  std::vector<std::string> data = m_data_m[header];
  validate(data);
  if (data.size() < 2) return "";
  std::string ret;
  std::string res = check_constant(data);
  if (!res.empty()) {
    ret += header + "=" + res + " conf:" + std::to_string(data.size());
    if (header[1] == 'd') {
      // add discovered constants. This is very likely to be a buffer size.
      // this is important for precondition generation
      m_discovered_constants.insert(atoi(res.c_str()));
    }
  }
  return ret;
}

RelKind check_against_constant(std::vector<std::string> data, int a) {
  assert(validate_number(data));
  std::vector<std::string> aa(data.size(), std::to_string(a));
  RelKind kind = check_relation_d(data, aa);
  return kind;
}

std::string Analyzer::checkTemplate(Formula *formula) {
  // 1. get the associated header columns
  // 2. validate those columns
  // 3. fit the formula to see if it matches
  // 4. optionally adjust the formula so that I have more flexibility
  //    - negate operator (need implement not)
  //    - add constant to each side
  assert(formula);
  std::string lhs = formula->GetLHS();
  std::vector<std::string> rhs = formula->GetRHS();
  std::vector<std::string> lhs_data;
  formula->ClearData();
  // LHS data
  if (m_data_m.count(lhs) == 1) {
    lhs_data = m_data_m[lhs];
  } else {
    // is constant
    assert(Formula::is_constant(lhs));
    // lhs is the data itself!
    lhs_data = std::vector<std::string>(m_data_dim, lhs);
  }
  formula->SetLHSData(lhs_data);
  // RHS data
  assert(rhs.size() <= 2);
  for (std::string r : rhs) {
    std::vector<std::string> r_data;
    if (m_data_m.count(r) == 1) {
      r_data = m_data_m[r];
    } else {
      assert(Formula::is_constant(r));
      r_data = std::vector<std::string>(m_data_dim, r);
    }
    formula->AddRHSData(r_data);
  }
  // validate the formula
  if (formula->Validate()) {
    return formula->ToString();
  }
  return "";
}

/**
 * Check the column, against a set of discovered constants, in m_discovered_constants
 * This is used for precondition generation, and the constants are discovered in invariants generation phase
 */
std::vector<std::string> Analyzer::checkDiscoveredConstants(std::string header) {
  std::vector<std::string> ret;
  if (m_discovered_constants.empty()) return ret;
  // std::cout << "m discovered constants: " << m_discovered_constants.size()  << "\n";
  // it should be numbers
  if (header[1] != 'd') return ret;
  std::vector<std::string> data = m_data_m[header];
  validate(data);
  if (data.size() < 2) return ret;
  for (int a : m_discovered_constants) {
    RelKind k = check_against_constant(data, a);
    if (k != RK_NA) {
      std::string res = rel_to_string(header, std::to_string(a), k);
      res += " conf: " + std::to_string(data.size());
      ret.push_back(res);
    }
  }
  return ret;
}


std::string Analyzer::checkRelation(std::string h1, std::string h2) {
  // std::cout << "checking relation: " << h1 << " vs. " << h2  << "\n";
  std::vector<std::string> d1 = m_data_m[h1];
  std::vector<std::string> d2 = m_data_m[h2];
  // only check when the type mtches
  // if (h1[1] != h2[1]) return RK_NA;
  validate(d1, d2);
  if (d1.size() < 2) return "";
  RelKind kind;
  if (h1[1] == 'd' && h2[1] == 'd') {
    kind = check_relation_d(d1, d2);
  } else if (h1[1] == 'p' && h2[1] == 'p') {
    kind = check_relation_p(d1, d2);
  } else {
    // std::cerr << "type does not match: " << h1[1] << ":" << h2[1] << "\n";
    kind = RK_NA;
  }
  std::string ret;
  if (kind != RK_NA) {
    ret = rel_to_string(h1, h2, kind);
    ret += " conf: " + std::to_string(d1.size());
  }
  return ret;
  // return check_relation(d1, d2, h1[1]);
}

std::string Analyzer::checkTransfer(std::string h1, std::string h2) {
  // std::cout << "checking transfer: " << h1 << " vs. " << h2  << "\n";
  std::vector<std::string> d1 = m_data_m[h1];
  std::vector<std::string> d2 = m_data_m[h2];
  // only compare when type matches
  // if (h1[1] != h2[1]) return "";
  validate(d1, d2);
  if (d1.size() < 2) return "";
  // check the distance is constant or not
  std::string dist;
  if (h1[1] == 'd' && h2[1] == 'd') {
    dist = check_const_dist_d(d1, d2);
  } else if (h1[1] == 'p' && h2[1] == 'p') {
    dist = check_const_dist_p(d1, d2);
  } else {
    // std::cerr << "type does not match: " << h1[1] << ":" << h2[1] << "\n";
  }
  std::string ret;
  if (!dist.empty()) {
    if (dist == "0") {
      ret = h2 + "=" + h1;
    } else {
      ret = h2 + "=" + h1 + "+" + dist;
    }
    ret += " conf:" + std::to_string(d1.size());
  }
  return ret;
}

/********************************
 * TESTs
 *******************************/

/**
 * Disabled because has a path
 */
TEST(AnalyzerTestCase, DISABLED_CSVTest) {
  Analyzer analyzer("/Users/hebi/tmp/b.csv", {});
  std::vector<std::string> invs = analyzer.GetInvariants();
  std::vector<std::string> pres = analyzer.GetPreConditions();
  std::vector<std::string> trans = analyzer.GetTransferFunctions();
  std::cout << "invariants:"  << "\n";
  for (std::string s : invs) {
    std::cout << s  << "\n";
  }
  std::cout << "pre conditions:"  << "\n";
  for (std::string s : pres) {
    std::cout << s  << "\n";
  }
  std::cout << "transfer functions:"  << "\n";
  for (std::string s : trans) {
    std::cout << s  << "\n";
  }
}
