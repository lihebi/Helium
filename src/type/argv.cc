#include "argv.h"
#include "utils/utils.h"
#include "helium_options.h"
#include "type.h"
#include <iostream>

ArgV* ArgV::m_instance=NULL;
InputSpec* ArgV::GetArgVInput() {
  if (m_argv.empty()) {
    generate();
  }
  assert(!m_argv.empty());
  std::string raw = std::to_string(m_argc) + " ";
  for (std::string v : m_argv) {
    raw += std::to_string(v.size() + 1) + " ";
    raw += v + " ";
  }
  std::string spec;
  spec += "{argc: " + std::to_string(m_argc) + ", argv.size: " + std::to_string(m_argv.size()) + "}";
  for (std::string s : m_argv) {
    spec += s + ", ";
  }
  
  return new InputSpec(spec, raw);
}

InputSpec* ArgV::GetArgCInput() {
  if (m_argv.empty()) {
    generate();
  }
  std::string raw;
  raw += std::to_string(m_argc);
  std::string spec;
  spec += "{argc: " + std::to_string(m_argc) + ", argv.size: " + std::to_string(m_argv.size()) + "}";
  return new InputSpec(spec, raw);;
}

void ArgV::generate() {
  // according to the opt, generate the argc and argv
  Type *strtype = TypeFactory::CreateType("char*");
  m_argv.clear();
  // argv0
  int argv0_strlen_max = HeliumOptions::Instance()->GetInt("test-input-max-argv0-strlen");
  int argv0_strlen = utils::rand_int(0, argv0_strlen_max+1);
  std::string argv0 = utils::rand_str(argv0_strlen);
  m_argv.push_back(argv0);
  
  // m_argv.push_back("helium_program");
  for (char c : m_bool_opt) {
    if (utils::rand_bool()) {
      m_argv.push_back("-" + std::string(1, c));
    }
  }
  for (char c : m_str_opt) {
    if (utils::rand_bool()) {
      m_argv.push_back("-" + std::string(1, c));
      // the string for the argument
      InputSpec *spec = strtype->GenerateRandomInput();
      m_argv.push_back(spec->GetRaw());
      delete spec;
    }
  }
  // MAGIC number 3
  int random_staff = utils::rand_int(0, 3);
  for (int i=0;i<random_staff;i++) {
    InputSpec *spec = strtype->GenerateRandomInput();
    m_argv.push_back(spec->GetRaw());
    // m_argv.push_back("Positional" + std::to_string(i));
    delete spec;
  }

  delete strtype;
  m_argc = m_argv.size();
}
