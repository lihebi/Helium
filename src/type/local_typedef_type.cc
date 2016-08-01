#include "type.h"
#include "resolver/snippet_db.h"
#include "type_helper.h"
#include "utils/utils.h"

LocalTypedefType::LocalTypedefType(int snippet_id, std::string raw, std::vector<std::string> dims, int token)
  : Type(raw, dims), m_snippet_id(snippet_id), m_token(token) {
  // FIXME right now just get the typedefine-d string
  // REMOVE the star!
  // and retain the dims
  // FIXME removing the '*'
  // utils::trim(raw);
  // while (raw.back() == '*') {
  //   raw.pop_back();
  //   utils::trim(raw);
  // }
  // std::cout << raw  << "\n";
  // assert(code.find("typedef") != std::string::npos);
  // assert(code.find(raw) != std::string::npos);
  std::string id = type::get_id(raw);
  std::string code = SnippetDB::Instance()->GetCode(snippet_id);
  m_code = code;
  if (code.find("typedef") == std::string::npos || code.find(id) == std::string::npos) {
    std::cerr << "Typedef seems wrong" << "\n";
    std::cout << "code:"  << "\n";
    std::cerr << code  << "\n";
    std::cout << "id:"  << "\n";
    std::cerr << id  << "\n";
    assert(false);
    m_next = NULL;
  }
  code = code.substr(code.find("typedef") + strlen("typedef"));
  code = code.substr(0, code.rfind(id));

  utils::trim(code);
  while (code.back() == '*') {
    code.pop_back();
    utils::trim(code);
  }
  m_next = TypeFactory::CreateType(code, dims, token);
}
LocalTypedefType::~LocalTypedefType() {
  if (m_next) {
    delete m_next;
  }
}


std::string LocalTypedefType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// HELIUM_TODO LocalTypedefType::GetInputCode " + var + "\n";
  return ret;
}

// virtual std::string GetInputCode(std::string var) override;
std::string LocalTypedefType::GetDeclCode(std::string var) {
  std::string ret;
  ret += m_raw + " " + var + DimensionSuffix() + ";\n";
  return ret;
}

std::string LocalTypedefType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// HELIUM_TODO LocalTypedefType::GetOutputCode " + var + "\n";
  return ret;
}
TestInput* LocalTypedefType::GetTestInputSpec(std::string var) {
  return NULL;
}
