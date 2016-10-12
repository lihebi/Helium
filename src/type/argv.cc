#include "argv.h"
#include "utils/utils.h"
#include "helium_options.h"
#include "type.h"

ArgV* ArgV::m_instance=NULL;
InputSpec* ArgV::GetArgVInput() {
  if (m_argv.empty()) {
    generate();
  }
  std::string raw = std::to_string(m_argc);
  std::string spec = "{}";
  return new InputSpec(raw, spec);
}


InputSpec* ArgV::GetArgCInput() {
  if (m_argv.empty()) {
    generate();
  }
  std::string raw;
  raw += std::to_string(m_argc);
  raw += " ";
  for (std::string s : m_argv) {
    raw += s + " ";
  }
  std::string spec = "{}";
  return new InputSpec(raw, spec);;
}

void ArgV::generate() {
  // according to the opt, generate the argc and argv
  Type *strtype = TypeFactory::CreateType("char*");
  m_argv.clear();
  std::vector<std::string> m_argv;
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
  int random_staff = utils::rand_int(0, 3);
  for (int i=0;i<random_staff;i++) {
    InputSpec *spec = strtype->GenerateRandomInput();
    m_argv.push_back(spec->GetRaw());
    delete spec;
  }
  m_argc = m_argv.size();
}
