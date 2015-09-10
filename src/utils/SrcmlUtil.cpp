#include "util/SrcmlUtil.hpp"
#include "util/ThreadUtil.hpp"

void SrcmlUtil::FileToXML(const std::string &filename, pugi::xml_document& doc) {
  std::string cmd;
  cmd = "src2srcml " + filename;
  std::string xml = ThreadUtil::Exec(cmd);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}
void SrcmlUtil::StringToXML(const std::string &code, pugi::xml_document& doc) {
  std::string cmd = "src2srcml -l C";
  std::string xml = ThreadUtil::Exec(cmd, code);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}
