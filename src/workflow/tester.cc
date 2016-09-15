#include "tester.h"
#include <gtest/gtest.h>
#include "utils/utils.h"
#include "config/options.h"

/**
 * @pram [in] output lines representing output. The format is xxx=yyy.
 * @return xxx:yyy maps
 */
std::map<std::string, std::string> get_header_value_map(std::string output) {
  std::map<std::string, std::string> ret;
  std::vector<std::string> lines = utils::split(output, '\n');
  for (std::string line : lines) {
    if (line.empty()) continue;
    // std::cout << line  << "\n";
    // assert(line.find("=") != std::string::npos);
    if (line.find("=") == std::string::npos) {
      // std::cerr << "The Line does not contain a =" << "\n";
      // std::cerr << line  << "\n";
      // assert(false);
      // FIXME sometimes the code we included from the program has output statements
      // So I just ignore such case
      // But, this may cause some hard to debug bugs
      // maybe it is a good idea to write this information to a log file for debugging
      continue;
    }
    std::string header = line.substr(0, line.find("="));
    utils::trim(header);
    std::string value = line.substr(line.find("=") + 1);
    utils::trim(value);
    ret[header] = value;
  }
  return ret;
}



/**
 * Invariants generation
 * This will parse the output.
 * The output format matters.
 * OK, I'm going to generate a CSV file, and use R to solve it.
 *
 * The output should be:
 * xxx = yyy
 * xxx != NULL
 *
 * Also, the output should be separate to test success and failure
 */
// void TestResult::GetInvariants() {
//   // 1. separate success and failure output
//   // std::vector<std::string> outputs = m_poi_output_success;
//   // 2. get all the headers (xxx)
//   std::vector<std::map<std::string, std::string> > header_value_maps;
//   for (std::string output : m_poi_output_success) {
//     std::map<std::string, std::string> m = get_header_value_map(output);
//     m["HELIUM_TEST_SUCCESS"] = "true";
//     header_value_maps.push_back(m);
//   }
//   for (std::string output : m_poi_output_failure) {
//     std::map<std::string, std::string> m = get_header_value_map(output);
//     m["HELIUM_TEST_SUCCESS"] = "false";
//     header_value_maps.push_back(m);
//   }
//   std::set<std::string> headers;
//   for (auto &m : header_value_maps) {
//     for (auto mm : m) {
//       headers.insert(mm.first);
//     }
//   }
//   // 3. generate the CVS file, all the unavailable fields should be marked as N/A
//   std::string csv;
//   assert(headers.size() > 0);
//   for (const std::string &header : headers) {
//     csv += header;
//     csv += ",";
//   }
//   csv.pop_back();
//   csv += "\n";

//   for (auto m : header_value_maps) {
//     // for every output, one line
//     for (const std::string &header : headers) {
//       if (m.count(header) == 1) {
//         csv += m[header] + ",";
//       } else {
//         csv += "N/A,";
//       }
//     }
//     csv.pop_back();
//     csv += "\n";
//   }
//   // 4. Call R script, generate result, in some format.
//   std::cout << csv  << "\n";
// }

void TestResult::GetInvariants() {
}

void TestResult::GetPreconditions() {
}

void TestResult::GetTransferFunctions() {
}

/**
 * Get the freed list, except the last, because it might be the address that cause the error.
 *
 * UPDATE Actaully I got a better idea:
 * 1. return the double freed address : for double free problem
 * 2. return the whole freed list : for use-after free problem
 * @return
 *   .first: the set of freed list
 *   .second: the addr string that is double freed.
 */
std::pair<std::set<std::string>, std::string> get_freed_list(std::string output) {
  std::set<std::string> freedlist;
  std::string double_freed;
  std::vector<std::string> lines = utils::split(output, '\n');
  for (std::string line : lines) {
    if (line.find("freedlist:") == 0) {
      std::string addr = line.substr(strlen("freedlist:"));
      assert(!addr.empty());
      utils::trim(addr);
      if (freedlist.count(addr) == 1) {
        double_freed = addr;
        // TODO assert this must be the last one, because the program already crashes
      } else {
        freedlist.insert(addr);
      }
    }
  }
  return {freedlist, double_freed};
}


/**
 * I'm using a new implementation of PrepareData and GenerateCSV
 * It will generate whatever it sees from the data, means:
 * 1. NO i_header and o_header
 * 2. NO HELIUM_POI checks
 */
void TestResult::PrepareData() {
  print_trace("TestResult::PrepareData");
  std::set<std::string> output_headers;
  // The final output will be written into m_header_value_maps, one test_suite per line.
  assert(m_poi_output.size() == m_test_suite.size());
  for (int i=0;i<(int)m_test_suite.size();i++) {
    std::map<std::string,std::string> test_suite_map;
    // output
    // This is called m_poi_output, actaully is not precise
    // It is the output of the whole program
    std::string output = m_poi_output[i].first;
    bool success = m_poi_output[i].second;


    // Freed-list
    // freedlist: 0x7fa56c000600
    std::pair<std::set<std::string>, std::string> p = get_freed_list(output);
    std::set<std::string> freedlist = p.first;
    std::string double_freed = p.second;


    std::map<std::string, std::string> m = get_header_value_map(output);
    test_suite_map.insert(m.begin(), m.end());
    // input
    std::string input;
    for (InputSpec *in : m_test_suite[i]) {
      if (in) {
        input += in->GetRaw();
      }
    }
    m = get_header_value_map(input);
    test_suite_map.insert(m.begin(), m.end());

    test_suite_map["HELIUM_TEST_SUCCESS"] = (success ? "true" : "false");
    if (!double_freed.empty()) {
      // TODO I use this name so that I don't need to change the analyzer
      // test_suite_map["HELIUM_DOUBLE_FREE"] = double_freed;
      test_suite_map["Op_helium_double_free"] = double_freed;
    }

    // TODO NOW check if the addresses are in freed list

    // use for the consistent behavior of all test_suites
    // The unavailable ones are marked as NA
    // The HELIUM_POI staff will appear NA if false, so FIXME
    for (auto m : test_suite_map) {
      m_headers.insert(m.first);
    }

    m_header_value_maps.push_back(test_suite_map);
  }
}

std::string TestResult::GenerateCSV() {
  print_trace("TestResult::GenerateCSV");
  std::string ret;
  for (const std::string &header : m_headers) {
    ret += header;
    ret += ",";
  }
  ret.pop_back();
  ret += "\n";
  // data
  for (auto m : m_header_value_maps) {
    assert(m.count("HELIUM_TEST_SUCCESS") == 1);
    for (const std::string &header : m_headers) {
      if (m.count(header) == 1) {
        ret += m[header] + ",";
      } else {
        // if the record does not contains the record, give is NA
        ret += "NA,";
      }
    }
    ret.pop_back();
    ret += "\n";
  }
  return ret;
}

/********************************
 * Resolving Query
 *******************************/


/**
 * Return the inversed version of op. > becomes <
 */
std::string inverse_op(std::string op) {
  if (op == "=") return "=";
  if (op == ">") return "<";
  if (op == ">=") return "<=";
  if (op == "<") return ">";
  if (op == "<=") return ">=";
  assert(false);
}

void BinaryFormula::Inverse() {
  std::string tmp = m_rhs;
  m_rhs = m_lhs;
  m_lhs = tmp;
  m_op = inverse_op(m_op);
}


BinaryFormula::BinaryFormula(std::string raw) : m_raw(raw) {
  std::string formula = raw.substr(0, raw.find("conf:"));
  std::string conf = raw.substr(raw.find("conf:") + 5);
  m_conf = atoi(conf.c_str());
  utils::trim(formula);
  // find the <, >, <=, >=, =
  size_t pos;
  int offset;
  if (formula.find("<=") != std::string::npos) {
    pos = formula.find("<=");
    offset = 2;
    m_op = "<=";
  } else if (formula.find(">=") != std::string::npos) {
    pos = formula.find(">=");
    offset = 2;
    m_op = ">=";
  } else if (formula.find("=") != std::string::npos) {
    pos = formula.find("=");
    offset = 1;
    m_op = "=";
  } else if (formula.find("<") != std::string::npos) {
    pos = formula.find("<");
    offset = 1;
    m_op = "<";
  } else if (formula.find(">") != std::string::npos) {
    pos = formula.find(">");
    offset = 1;
    m_op = ">";
  } else {
    assert(false);
  }
  m_lhs = formula.substr(0, pos);
  m_rhs = formula.substr(pos + offset);
  utils::trim(m_lhs);
  utils::trim(m_rhs);
  if (!is_var(m_lhs)) {
    std::string tmp = m_lhs;
    m_lhs = m_rhs;
    m_rhs = tmp;
  }
}



TEST(SegTestCase, BinaryFormulaTest) {
  BinaryFormula bf("Od_sizeof(tempname)=1024 conf:18");
  EXPECT_EQ(bf.GetLHS(), "Od_sizeof(tempname)");
  EXPECT_EQ(bf.GetRHS(), "1024");
  EXPECT_EQ(bf.GetOP(), "=");
  EXPECT_EQ(bf.GetConf(), 18);

  BinaryFormula bf2("Id_strlen(argv[1])<=1024");
  EXPECT_EQ(bf2.GetLHSVar(), "argv");
  std::set<std::string> ss = bf2.GetVars();
  // for (auto &s : ss) {
  //   std::cout << s  << "\n";
  // }
  ASSERT_EQ(ss.size(), 1);
  EXPECT_EQ(*ss.begin(), "argv");
}

/**
 * Get the variables.
 * for Od_strlen(*fileptr[1]), it will be fileptr along
 * Since this is binary, the return set is at most of size 2
 */
std::set<std::string> BinaryFormula::GetVars() {
  std::set<std::string> ret;
  std::string tmp;
  tmp = getVar(m_lhs);
  if (!tmp.empty()) {
    ret.insert(tmp);
  }
  tmp = getVar(m_rhs);
  if (!tmp.empty()) {
    ret.insert(tmp);
  }
  return ret;
}


std::string BinaryFormula::getVar(std::string s) {
  // remove prefix
  std::cout << s  << "\n";
  // assert(s.size() > 3);
  // FIXME constant, say, 50
  if (s.size() < 3) return "";
  // assert(s[2] == '_');
  if (s[2] != '_') return "";
  s = s.substr(3);
  // remove strlen, sizeof
  // FIXME multiple sizeof?
  if (s.find("sizeof") != std::string::npos) {
    s = s.substr(s.find("sizeof") + strlen("sizeof"));
  }
  if (s.find("strlen") != std::string::npos) {
    s = s.substr(s.find("strlen") + strlen("strlen"));
  }
  // remove ()
  while (s.find('(') != std::string::npos) {
    s.erase(s.find('('), 1);
  }
  while (s.find(')') != std::string::npos) {
    s.erase(s.find(')'), 1);
  }
  // remove [xxx]
  if (s.find('[') != std::string::npos) {
    s = s.substr(0, s.find('['));
  }
  // remove .xxx
  if (s.find('.') != std::string::npos) {
    s = s.substr(0, s.find('.'));
  }
  // remove + ...
  if (s.find('+') != std::string::npos) {
    s = s.substr(0, s.find('+'));
  }
  utils::trim(s);
  return s;
}


/**
 * Od_sizeof(tempname)=1024 conf:18
 * Od_sizeof(tempname)>=Od_strlen(*fileptr) conf: 18
 */
BinaryFormula* get_key_inv(std::vector<BinaryFormula*> invs) {
  // loop through the invs, and split into two sets: constant assignment, and other
  assert(!invs.empty());
  BinaryFormula *ret = NULL;
  std::vector<BinaryFormula*> cons;
  for (BinaryFormula *bf : invs) {
    if (bf->GetOP() == "=" && utils::is_number(bf->GetRHS())) {
      cons.push_back(bf);
    } else {
      ret = bf;
    }
  }
  if (!ret) {
    // FIXME warning?
    return *invs.begin();
  }
  for (BinaryFormula *bf : cons) {
    if (bf->GetLHS() == ret->GetRHS()) {
      ret->UpdateRHS(bf->GetRHS());
    }
    if (bf->GetLHS() == ret->GetLHS()) {
      ret->UpdateLHS(bf->GetRHS());
      ret->Inverse();
    }
  }
  return ret;
}

/**
 * Derive inv from pres and trans.
 * If cannot derive, return an empty set.
 * Otherwise return the used pre-conditions.
 * This return value is used to see if the variables are entry point.
 * TODO multiple precondition
 * TODO record the transfer function used.
 *
 * inv: Od_sizeof(tempname)>=Od_strlen(*fileptr) conf: 12
 * trans: Od_strlen(*fileptr)=Id_strlen(argv[1]) conf:12
 * pres: Id_strlen(argv[1])<=1024 conf: 12
 */
BinaryFormula* derive_key_inv(std::vector<BinaryFormula*> pres, std::vector<BinaryFormula*> trans, BinaryFormula *inv) {
  // std::cout << "drive_key_inv"  << "\n";
  // std::cout << "pres:"  << "\n";
  // for (BinaryFormula *bf : pres) {
  //   std::cout << bf->dump()  << "\n";
  // }
  // std::cout << "trans:"  << "\n";
  // for (BinaryFormula *bf : trans) {
  //   std::cout << bf->dump()  << "\n";
  // }
  // std::cout << "inv:"  << "\n";
  // std::cout << inv->dump()  << "\n";
  // randomly pair the trans and pres, to see if inv can be generated
  for (BinaryFormula *pre : pres) {
    for (BinaryFormula *tran : trans) {
      BinaryFormula tmp(*pre);
      if (tran->GetRHS() == tmp.GetLHS()) {
        tmp.UpdateLHS(tran->GetLHS());
      }
      if (tran->GetRHS() == tmp.GetRHS()) {
        tmp.UpdateRHS(tran->GetLHS());
      }
      // compare_formula(&tmp, inv);
      if (tmp.GetLHS() == inv->GetLHS() && tmp.GetRHS() == inv->GetRHS() && tmp.GetOP() == inv->GetOP()) {
        return pre;
      }
      if (tmp.GetLHS() == inv->GetRHS() && tmp.GetRHS() == inv->GetLHS() && inverse_op(tmp.GetOP()) == inv->GetOP()) {
        return pre;
      }
    }
  }
  return NULL;
}


BinaryFormula *merge(BinaryFormula *fm1, BinaryFormula *fm2) {
  if (!fm1) return NULL;
  if (!fm2) return NULL;
  std::string lhs1 = fm1->GetLHS();
  std::string rhs1 = fm1->GetRHS();
  std::string lhs2 = fm2->GetLHS();
  std::string rhs2 = fm2->GetRHS();
  if (lhs1.empty() || rhs1.empty()) return NULL;
  if (lhs2.empty() || rhs2.empty()) return NULL;
  if (!BinaryFormula::is_var(rhs1)) {
    if (lhs1 == lhs2 || lhs1 == rhs2) {
      BinaryFormula *fm = new BinaryFormula(*fm2);
      fm->Update(lhs1, rhs1);
      return fm;
    }
  } else if (!BinaryFormula::is_var(rhs2)) {
    if (lhs2 == lhs1 || lhs2 == rhs1) {
      BinaryFormula *fm = new BinaryFormula(*fm1);
      fm->Update(lhs2, rhs2);
      return fm;
    }
  }
  return NULL;
}
