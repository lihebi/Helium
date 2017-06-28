#include "helium/utils/XMLDocReader.h"
#include "helium/utils/Utils.h"
#include "helium/utils/XMLNodeHelper.h"
#include "helium/utils/ThreadUtils.h"
#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// I should try my best to fill the filename here
XMLDoc* XMLDocReader::CreateDocFromString(const std::string &code, std::string filename) {
  std::string cmd;
  // cmd = "helium-srcml --position - ";
  cmd = "srcml-client.py -";
  // FIXME
  // if (!filename.empty()) {
  //   cmd += " -f " + filename;
  // }
  // std::string cmd = "srcml -lC";
  std::string xml = utils::exec_in(cmd.c_str(), code.c_str(), NULL);
  pugi::xml_document *doc = new pugi::xml_document();
  doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  return doc;
}

XMLDoc* XMLDocReader::CreateDocFromFile(std::string filename) {
  std::string cmd;
  // cmd = "helium-srcml --position " + filename;
  cmd = "srcml-client.py " + filename;
  std::string xml = utils::exec(cmd.c_str(), NULL);
  pugi::xml_document *doc = new pugi::xml_document();
  doc->load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  return doc;
}
