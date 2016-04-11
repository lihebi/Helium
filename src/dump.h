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
private:
  ExpASTDump() {}
  static ExpASTDump *m_instance;
};

#endif /* DUMP_H */
