#include "failure_point.h"
#include "utils/utils.h"
#include <iostream>

/**
 * read the first line
 * split into three parts
 * simple filename, line number, type
 */
FailurePoint *FailurePointFactory::CreateFailurePoint(std::string file) {
  if (!utils::file_exists(file)) {
    return NULL;
  }
  std::string content = utils::read_file(file);
  std::vector<std::string> sp = utils::split(content);
  assert(sp.size() > 2);


  std::string filename = sp[0];
  if (filename.empty()) return NULL;
  int linum = -1;
  try {
    linum = std::stoi(sp[1]);
  } catch (std::invalid_argument e) {
    std::cerr << "EE: error parsing poi file line number"  << "\n";
    return NULL;
  }
  std::string type = sp[2];
  if (type != "loop" && type != "stmt") {
    return NULL;
  }
  return new FailurePoint(file, filename, linum, type);
}

/**
 * Parse whole poi file.
 * TODO use a format table parser
 | benchmark                 | file               | line | type | bug-type |
 |---------------------------+--------------------+------+------+----------|
 | cabextract-1.2            | mszipd.c           |  353 | loop |          |
 */
FailurePoint *FailurePointFactory::CreateFailurePoint(std::string file, std::string name) {
  if (!utils::file_exists(file)) {
    return NULL;
  }
  std::string content = utils::read_file(file);
  std::vector<std::string> lines = utils::split(content, '\n');
  for (std::string line : lines) {
    std::vector<std::string> components = utils::split(line, '|');
    // FIXME the first is empty?
    // FIXME what is there's one empty in the middle?
    // FIXME in the end?
    if (components.size() < 6) {
      continue;
    }
    std::string bench = components[1];
    std::string filename = components[2];
    std::string linum_str = components[3];
    std::string type = components[4];
    std::string bug_type = components[5];
    utils::trim(bench);
    utils::trim(filename);
    utils::trim(linum_str);
    utils::trim(type);
    utils::trim(bug_type);



    
    // std::cout << bench << ":" << file << ":" << linum_str  << "\n";
    if (bench == name) {
      std::cout << "found POI: " << filename << ":" << linum_str << " " << type  << "\n";
      int linum = -1;
      try {
        linum = std::stoi(linum_str);
      } catch (std::invalid_argument e) {
        std::cout << "Error parsing poi file "
                  << filename << ": linum_str is not a number"  << "\n";
        return NULL;
      }
      if (filename.empty()
          || linum == 0
          || type.empty()
          ) {
        std::cerr << "EE: whole poi format error."  << "\n";
        return NULL;
      }
      return new FailurePoint(file, filename, linum, type);
    }
  }
  return NULL;
}


// TEST(HeliumTestCase, WholePOITest) {
//   std::string s = "| xfd | fds | | xxfd | |";
//   // the test shows: the first is empty, the last empty is omitted (in this case only one last empty string)
//   std::vector<std::string> v = utils::split(s, '|');
//   for (std::string s : v) {
//     std::cout << "COMP: " << s  << "\n";
//   }
// }
