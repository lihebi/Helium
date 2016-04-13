#ifndef DUMP_H
#define DUMP_H
#include "common.h"
/**
 * echo "benchmark, file count, func count, compile success, compile error, build rate, time(s)"
 * ALso
 * Now I want to dump the max,min,mean (leafsize, snippet size) of the successfully built code.
 */
class ExpASTDump {
public:
  static ExpASTDump *Instance() {
    if (!m_instance) {
      m_instance = new ExpASTDump();
    }
    return m_instance;
  }
  std::string dump();
  std::string GetHeader();
  /**
   * Data
   */
  std::string benchmark;
  int file_count = 0;
  int func_count = 0;
  int compile_success = 0;
  int compile_error = 0;
  double time = 0;


  std::vector<int> suc_leaf_size;
  std::vector<int> err_leaf_size;
  std::vector<int> suc_snippet_size;
  std::vector<int> err_snippet_size;
  void AppendData();


  /**
   * Now I want this kind of data

   leafsize, snippet_size, build_or_not
   12, 34, true
   15, 76, false
   */
  

private:
  ExpASTDump() {}
  static ExpASTDump *m_instance;
};

class BuildRatePlotDump {
public:
  struct Data {
    Data(int leaf, int snippet, bool build)
      : leaf_size(leaf), snippet_size(snippet), build_or_not(build) {}
    int leaf_size = -1;
    int snippet_size = -1;
    bool build_or_not = false;
  };

  void Push(int leaf_size, int snippet_size, bool build_or_not) {
    struct Data data = {leaf_size, snippet_size, build_or_not};
    m_data.push_back(data);
  }
  static std::string GetHeader() {
    return "leaf_size, snippet_size, build_or_not";
  }
  std::string dump() {
    std::string ret;
    ret += GetHeader() + "\n";
    for (struct Data data : m_data) {
      ret += std::to_string(data.leaf_size) + ", "
        + std::to_string(data.snippet_size) + ", "
        + (data.build_or_not?"true":"false") + "\n";
    }
    return ret;
  }
  static BuildRatePlotDump *Instance() {
    if (!m_instance) {
      m_instance = new BuildRatePlotDump();
    }
    return m_instance;
  }
private:
  std::vector<struct Data> m_data;
  static BuildRatePlotDump *m_instance;
};


#endif /* DUMP_H */
