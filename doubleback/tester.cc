#include "tester.h"
#include <fstream>
#include "utils.h"
#include "config.h"

#include <iostream>

using namespace utils;

static int
myrand(int low, int high) {
  double d = rand();
  d /= RAND_MAX;
  return low + (high-low)*d;
}

Tester::Tester(const std::string &dir, Segment *seg)
  : m_dir(dir), m_seg(seg), m_success(false) {
  srand(time(0));
  // the first random number is highly related to the time, so we don't use it
  rand();
  // one defect per file!
  m_segment_begin_line = utils::get_line_number(dir+"/main.c", "@HeliumSegmentBegin");
  m_segment_end_line = utils::get_line_number(dir+"/main.c", "@HeliumSegmentEnd");
}

Tester::~Tester() {}

std::string
get_input_by_spec(std::string spec) {
  std::vector<std::string> specs = split(spec, ',');
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
  // get input specification
  VariableList inv = m_seg->GetInputVariables();
  std::string text;
  // for (auto it=inv.begin();it!=inv.end();it++) {
  //   std::string input_spec = (*it)->GetInputSpecification();
  //   text += get_input_by_spec(input_spec);
  // }
  // random & pair
  return text;
}

enum ValgrindKind parse_valgrind(std::string filename, int begin, int end) {
  // std::cout <<"parse valgrind: " << filename  << "\n";
  if (!utils::is_file(filename)) {
    std::cerr << "valgrind filename " << filename << "doesnot exist\n";
    assert(false);
  }
  /*******************************
   ** parse valgrind
   *******************************/
  pugi::xml_document val_doc;
  // std::cout <<"segline:"<<m_defect_line  << "\n";
  val_doc.load_file(filename.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  std::set<int> lines;
  for (auto error : val_doc.select_nodes("//error")) {
    pugi::xml_node error_node = error.node();
    std::string kind = error_node.child_value("kind");
    if (kind != "InvalidRead" && kind != "InvalidWrite") continue;
    for (pugi::xml_node frame : error_node.child("stack").children("frame")) {
      std::string filename = frame.child_value("file");
      std::string line = frame.child_value("line");
      if (filename == "main.c") {
        lines.insert(atoi(line.c_str()));
        // utils::print(kind+":"+line, utils::CK_Cyan);
      }
    }
  }
  if (lines.empty()) {
    return VK_Success; // m_val_success ++;
  } else {
    for (int line : lines) {
      if (line >= begin && line <= end) {
        return VK_CorrectFailure;
      }
    }
    return VK_WrongFailure;
  }
}

// void run_valgrind(std::string dir, std::string input_filename, int begin, int end, TestResult &result) {
//   /*******************************
//    ** valgrind
//    *******************************/
//   std::string valgrind_output = dir+"/valgrind.out.xml";
//   std::string executable = dir+"/a.out";
//   std::string cmd =
//     "valgrind --xml=yes --xml-file="
//     + valgrind_output
//     + " " + executable + "< " + input_filename + " 2> /dev/null";
//   // cmd = "{ " + cmd + "; } >/dev/null 2>&1";
//   // int status=0;
//   utils::exec(cmd.c_str(), NULL);
//   // result.return_code = status;
//   result.vk = parse_valgrind(valgrind_output, begin, end);
// }

void run(std::string dir, std::string input_filename, TestResult &result) {
  std::string executable = dir+"/a.out";
  std::string cmd = executable + "< " + input_filename +  " 2>/dev/null";
  // redirect segment fault, which is not the output of this program, but that of the process that runs ths one.
  cmd = "{ " + cmd + "; } 2>/dev/null";
  // std::cout <<cmd  << "\n";
  int status=0;
  std::string output = utils::exec(cmd.c_str(), &status);
  // if (!output.empty()) std::cout <<output  << "\n";
  // std::cout <<output  << "\n";
  // this actually is not a caution.
  // See the report for detailed discussion.
  // basically it may because the input just doesn't execute the segment, and still terminate gracefully.
  // an example is: if (strlen(from)<bound) strcpy(to, from)
  // a string `from` longer than bound will not pass the checking, thus avoid the buggy strcpy. Which is GOOD.
  // if (output.find("@HeliumBeforeSegment") == std::string::npos && status==0) {
  //   // so i need to really implement checking of output for HeliumBeforesegment and HeliumAfterSegment
  //   std::cout <<"caution";
  // }
  result.return_code = status;
}


void
Tester::Test() {
  // int count1=0;
  // int count2=0;
  // CAUTION length_vec is only used for store length
  std::vector<std::string> length_vec;
  /*******************************
   ** Running tests
   *******************************/
  std::string executable = m_dir+"/a.out";
  std::string test_dir = m_dir + "/test";
  utils::create_folder(test_dir);
  int test_number = Config::Instance()->GetInt("test-number");
  for (int i=0;i<test_number;i++) { // 10 tests each
    /*******************************
     ** constructing input
     *******************************/
    std::string input;
    VariableList inv = m_seg->GetInputVariables();
    // CAUTION length is used only for stack-vs-heap experiment
    // std::string length = "";
    for (Variable v : inv) {
      std::string text = get_random_input(v.GetType());
      input += text;
      // length = utils::split(text)[0];
    }
    // std::cout <<input  << "\n";
    // std::cout <<"input:"  <<input<< "\n";
    std::string input_filename = test_dir + "/test-" + std::to_string(i) + "-input.txt";
    utils::write_file(input_filename, input);
    TestResult tr;
    run(m_dir, input_filename, tr);
    // run_valgrind(m_dir, input_filename, m_segment_begin_line, m_segment_end_line, tr);
    m_results.push_back(tr);
    if (tr.return_code == 0) {
      std::cout <<'.';
    } else {
      std::cout <<'x';
      // count1++;
    }
    // if (tr.vk == VK_CorrectFailure) {
    //   valgrind_fail++;
    //   if (tr.return_code == 0) {
    //     vs.push_back(length);
    //   }
    // }

    // if (atoi(length.c_str()) > 100) {
    //   count2++;
    //   if (tr.return_code == 0) {
    //     length_vec.push_back(length);
    //   }
    // }
    
    flush(std::cout);
  }
  std::cout << "\n";
  // std::cout <<count1<<"," << count2  << "\n";
  // for (std::string& s : length_vec) {
  //   std::cout <<s  << " ";
  // }
}

void
Tester::WriteCSV() {
  // std::cout << utils::PURPLE <<"val_success: " << m_val_success  << "\n";
  // std::cout <<"val_correct_failure: " << m_val_correct_failure  << "\n";
  // std::cout <<"val_wrong_failure: " << m_val_wrong_failure << "\n";
  // std::cout <<"return success: " << m_return_success  << "\n";
  // std::cout <<"return fail: " << m_return_failure  << "\n";
  // std::cout << utils::RESET<< "\n";
  // mylog(m_val_success);
  // mylog(",");
  // mylog(m_val_correct_failure);
  // mylog(",");
  // mylog(m_val_wrong_failure);
  // mylog(",");
  // mylog(m_return_success);
  // mylog(",");
  // mylog(m_return_failure);
  // mylog("\n");
  
  // FILE *fp = fopen((m_dir+ "/result.txt").c_str(), "w");
  // fprintf(fp, "ID, return code, valgrind(0:suc 1:cf 2:wf)\n");
  // for (size_t i=0;i<m_results.size();i++) {
  //   fprintf(fp, "%lu,%d,%d\n",
  //           i,
  //           m_results[i].return_code,
  //           m_results[i].vk
  //           );
  // }
  static int idx = 0;
  idx++;
  if (idx == 1) {
    FILE *fp = fopen("/tmp/helium-result.csv", "w");
    fprintf(fp, "ID, runtime failure, total\n");
    fclose(fp);
  }

  FILE *fp = fopen("/tmp/helium-result.csv", "a");
  // ID, runtime failure, total
  size_t size = m_results.size();
  int count=0;
  for (TestResult &tr : m_results) {
    if (tr.return_code != 0) count++;
  }
  fprintf(fp, "%d,%d,%lu\n", idx, count, size);
  fclose(fp);
}

