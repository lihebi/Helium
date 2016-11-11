#ifndef IO_HELPER_H
#define IO_HELPER_H

#include "common.h"
#include "utils/string_utils.h"


class IOHelper {
public:
  static IOHelper *Instance() {
    if (!m_instance) {
      m_instance=new IOHelper();
    }
    return m_instance;
  }
  /**
   * @param [in] the type raw string
   * @return a string that is capable of being a function name in C
   */
  static std::string ConvertTypeStr(std::string raw) {
    utils::trim(raw);
    utils::replace(raw, " ", "_");
    utils::replace(raw, "*", "star");
    utils::replace(raw, "[", "L");
    utils::replace(raw, "]", "J");
    return raw;
  }
  static std::string GetInputCall(std::string key, std::string var) {
    return "input_"+key+"(&"+var+");\n";
  }
  static std::string GetOutputCall(std::string key, std::string var, std::string name) {
    const char *format=R"prefix(
output_%s(%s,%s);
)prefix";
    char buf[BUFSIZ];
    sprintf(buf, format, key.c_str(), var.c_str(), name.c_str());
    std::string ret=buf;
    utils::trim(ret);
    ret+="\n";
    return ret;
  }
  bool Has(std::string s) {
    if (m_input.count(s)==1) return true;
    else return false;
  }
  void Add(std::string key, std::string input, std::string output) {
    m_input[key] = input;
    m_output[key] = output;
  }
private:
  IOHelper();
  ~IOHelper();
  static IOHelper *m_instance;
  // from type string to data
  std::map<std::string, std::string> m_input;
  std::map<std::string, std::string> m_output;
};




#endif /* IO_HELPER_H */
