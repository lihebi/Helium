#include "util/SrcmlUtil.hpp"
#include "util/ThreadUtil.hpp"

void SrcmlUtil::File2XML(const std::string &filename, pugi::xml_document& doc) {
  std::cout << "[SrcmlUtil::File2XML]" << std::endl;
  std::string cmd;
  cmd = "src2srcml " + filename;
  std::string xml = ThreadUtil::Exec(cmd.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  std::cout << "[SrcmlUtil::File2XML] end" << std::endl;
}
void SrcmlUtil::String2XML(const std::string &code, pugi::xml_document& doc) {
  std::string cmd = "src2srcml -lC";
  std::string xml = ThreadUtil::Exec(cmd.c_str(), code.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}
