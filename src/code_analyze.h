#ifndef CODE_ANALYZE_H
#define CODE_ANALYZE_H

#include "common.h"

class CodeAnalyzer {

public:
  CodeAnalyzer(std::string folder) : m_folder(folder) {}
  ~CodeAnalyzer() {}
  void Compute();
private:
  std::string m_folder;
};

#endif /* CODE_ANALYZE_H */
