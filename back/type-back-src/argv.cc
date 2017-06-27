#include "helium/type/argv.h"
#include "helium/utils/utils.h"
#include "helium/utils/helium_options.h"
#include "helium/type/type.h"
#include <iostream>


void ArgV::clear() {
  m_argv0.clear();
  m_bool_input.clear();
  m_str_input.clear();
  m_propositional_input.clear();
}

void ArgV::SetOpt(std::string opt) {
  m_opt = opt;
  for (int i=opt.length()-1;i>=0;i--) {
    if (opt[i] == ':') {
      i--;
      m_str_opt.insert(opt[i]);
    } else {
      m_bool_opt.insert(opt[i]);
    }
  }
}



ArgV* ArgV::m_instance=NULL;
InputSpec* ArgV::GetArgVInput() {
  if (m_argv0.empty()) {
    generate();
  }

  // argv0
  // bool options
  // str options
  // propositional options

  std::string raw;
  std::string spec;
  int argc = 1;
  argc += m_bool_input.size();
  argc += m_str_input.size() * 2;
  argc += m_propositional_input.size();
  raw += std::to_string(argc) + " ";

  raw += std::to_string(m_argv0.size() + 1) + " ";
  raw += m_argv0 +  " ";

  spec += "argc: " + std::to_string(argc) + " ";
  spec += "argv0.size: " + std::to_string(m_argv0.size()) + ", ";

  spec += "bool_option: ";
  for (char c : m_bool_input) {
    raw += "5 ";
    raw += "-" + std::string(1, c) + " ";
    spec += c;
    spec += ",";
  }

  spec += "str_option: ";
  for (auto m : m_str_input) {
    char c = m.first;
    InputSpec *tmp = m.second;
    raw += "5 ";
    raw += "-" + std::string(1, c) + " ";
    raw += tmp->GetRaw() + " ";

    spec += c;
    spec += ": ";
    spec += tmp->GetSpec();
    spec += ",";
  }

  spec += "propositional: ";
  for (InputSpec *tmp : m_propositional_input) {
    raw += tmp->GetRaw() + " ";
    spec += tmp->GetSpec();
  }
  
  // spec += "{argc: " + std::to_string(m_argc) + ", argv.size: " + std::to_string(m_argv.size()) + "}";

  // for (std::string s : m_argv) {
  //   spec += s + ", ";
  // }
  
  return new InputSpec(spec, raw);
}

InputSpec* ArgV::GetArgCInput() {
  if (m_argv0.empty()) {
    generate();
  }
  std::string raw;
  int argc = 1;
  argc += m_bool_input.size();
  argc += m_str_input.size() * 2;
  argc += m_propositional_input.size();
  raw += std::to_string(argc);
  std::string spec;
  spec += "{argc: " + std::to_string(argc) + "}";
  return new InputSpec(spec, raw);;
}

void ArgV::generate() {
  // according to the opt, generate the argc and argv
  Type *strtype = TypeFactory::CreateType("char*");
  // argv0
  int argv0_strlen_max = HeliumOptions::Instance()->GetInt("test-input-max-argv0-strlen");
  int argv0_strlen = utils::rand_int(1, argv0_strlen_max+1);
  m_argv0 = utils::rand_str(argv0_strlen);
  
  // m_argv.push_back("helium_program");
  for (char c : m_bool_opt) {
    if (utils::rand_bool()) {
      m_bool_input.push_back(c);
    }
  }
  for (char c : m_str_opt) {
    if (utils::rand_bool()) {
      m_str_input[c] = strtype->GenerateRandomInput();
    }
  }
  // MAGIC number 3
  int random_staff = utils::rand_int(0, 3);
  for (int i=0;i<random_staff;i++) {
    InputSpec *spec = strtype->GenerateRandomInput();
    m_propositional_input.push_back(spec);
    // m_argv.push_back(spec->GetRaw());
    // m_argv.push_back("Positional" + std::to_string(i));
    // delete spec;
  }

  delete strtype;
  // m_argc = m_argv.size();
}
