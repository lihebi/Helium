#include "code_analyze.h"
#include "utils/utils.h"
#include <iostream>

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


void CodeAnalyzer::Compute() {
  // read files in m_folder/tests/
  // in-x.txt
  /// out-x.txt

  std::vector<std::string> files = utils::get_files(m_folder + "/tests/");;
  

  std::vector<std::pair<std::string, std::string> > pairs;
  std::map<std::string, std::pair<std::string, std::string> > filemap;
  // according to file names, divide them into different pairs
  for (std::string file : files) {
    std::string simple = file.substr(file.rfind('/')+1);
    std::string inout = simple.substr(0, simple.find('-'));
    std::string num = simple.substr(simple.find('-'), simple.find('.'));
    if (inout == "in") {
      filemap[num].first = file;
    } else {
      filemap[num].second = file;
    }
  }
  for (auto m : filemap) {
    pairs.push_back(m.second);
  }


  // generate a CSV file to hook previous analysis engine

  std::vector<std::map<std::string, std::string> > data;
  std::set<std::string> headers;
  for (auto p : pairs) {
    std::string in_file = p.first;
    std::string out_file = p.second;
    std::string in = utils::read_file(in_file);
    std::string out = utils::read_file(out_file);
    
    std::map<std::string, std::string> tmpmap = get_header_value_map(out);
    for (auto m : tmpmap) {
      headers.insert(m.first);
    }

    data.push_back(tmpmap);
  }

  // generate CSV
  std::string csv;
  for (const std::string &header : headers) {
    csv += header;
    csv += ",";
  }
  csv.pop_back();
  csv += "\n";
  for (auto m : data) {
    for (const std::string &header : headers) {
      if (m.count(header) == 1) {
        csv += m[header] + ",";
      } else {
        // if the record does not contains the record, give is NA
        csv += "NA,";
      }
    }
    csv.pop_back();
    csv += "\n";
  }
  // write csv into file
  utils::write_file(m_folder + "/io.csv", csv);
  std::cout << "CSV file wrote to " << m_folder + "/io.csv"  << "\n";
}
