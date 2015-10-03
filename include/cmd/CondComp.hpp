#ifndef __COND_COMP_HPP__
#define __COND_COMP_HPP__

#include <string>
#include <set>
#include <vector>

class CondComp {
public:
  CondComp(const std::string &folder);
  ~CondComp() {}
  void Run();
private:
  void getUsedMacros();
  void getDefinedMacros();
  int getAction(const std::string& line);
  bool process(std::vector<std::string>& lines);
  std::string m_folder;
  std::vector<std::string> m_files;
  std::set<std::string> m_macros;
  std::set<std::string> m_defined_macros;
};

#endif
