#include "Tester.hpp"
#include "util/StringUtil.hpp"
#include "util/ThreadUtil.hpp"
#include <fstream>

static int
myrand(int low, int high) {
  double d = rand();
  d /= RAND_MAX;
  return low + (high-low)*d;
}

Tester::Tester(const std::string &executable, std::shared_ptr<SegmentProcessUnit> seg_unit)
: m_executable(executable), m_seg_unit(seg_unit), m_success(false) {
  std::cout << "[Tester::Tester] " << executable << std::endl;
  srand(time(0));
  // the first random number is highly related to the time, so we don't use it
  rand();
}

Tester::~Tester() {}

std::string
get_input_by_spec(std::string spec) {
  std::vector<std::string> specs = StringUtil::Split(spec, ',');
  std::string text;
  int size = 1;
  if (specs.size() == 0) return "";
  // array
  if (specs[0] == "size") {
    size = myrand(1, 10);
    text += std::to_string(size);
    text += " ";
    specs.erase(specs.begin());
  }
  for (int i=0;i<size;i++) {
    for (auto it=specs.begin();it!=specs.end();it++) {
      if (it->find("_") == std::string::npos) continue;
      int low = atoi(it->substr(0, it->find("_")).c_str());
      int high = atoi(it->substr(it->find("_")+1).c_str());
      text += std::to_string(myrand(low, high)) + " ";
    }
  }
  return text;
}

std::string
Tester::generateInput() {
  std::cout << "[Tester::generateInput]" << std::endl;
  // get input specification
  std::set<std::shared_ptr<Variable> > inv = m_seg_unit->GetInputVariables();
  std::string text;
  for (auto it=inv.begin();it!=inv.end();it++) {
    std::string input_spec = (*it)->GetInputSpecification();
    std::cout << "input_spec: " << input_spec << std::endl;
    text += get_input_by_spec(input_spec);
  }
  // random & pair
  return text;
}

void
Tester::Test() {
  std::cout << "[Tester::Test]" << std::endl;
  // generate input
  std::string input = generateInput();
  std::cout << "input:" << std::endl;
  std::cout << input << std::endl;
  // run program
  std::string cmd;
  // right now, hard code to timeout 2 seconds
  std::string result = ThreadUtil::Exec(m_executable.c_str(), input.c_str(), NULL, 2);
  // std::cout << "result: " << std::endl;
  // std::cout << result << std::endl;
  // get output
  std::set<std::shared_ptr<Variable> > outv = m_seg_unit->GetOutputVariables();
  int size = outv.size();
  std::vector<std::string> results = StringUtil::Split(result);
  std::string outcsv = Config::Instance()->GetOutputFolder() + "/out.csv";
  std::ofstream os;
  os.open(outcsv);
  if (os.is_open()) {
    int count = 0;
    for (auto it=outv.begin();it!=outv.end();it++) {
      os << (*it)->GetName();
      if (++count<size) {
        os << ",";
      }
    }
    os << "\n";
    count = 0;
    for (auto it=results.begin();it!=results.end();it++) {
      count++;
      os << *it;
      if (count ==size) {
        count = 0;
        os << '\n';
      } else {
        os << ',';
      }
    }
    m_success = true;
    os.close();
  }
}

bool Tester::Success() {
  // TODO
  return m_success;
}

TestResult Tester::GetOutput() {
  return m_test_result;
}

void
Tester::WriteCSV() {}
