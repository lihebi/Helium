#include "type.h"
#include "utils/utils.h"
#include "helium_options.h"


ArgCVType::ArgCVType(std::string getopt_str) {
  // "achtvf:"
  // only support single colon
  for (int i=getopt_str.length()-1;i>=0;i--) {
    if (getopt_str[i] == ':') {
      i--;
      m_bools.push_back(getopt_str[i]);
    } else {
      m_named_args.push_back(getopt_str[i]);
    }
  }
  m_argv = TypeFactory::CreateType("char**");
  m_argc = TypeFactory::CreateType("int");
}
ArgCVType::~ArgCVType() {}

std::string ArgCVType::GetDeclCode(std::string var) {
  // ignoring var!
  std::string ret;
  // ret += "// ArgCVType::GetDeclCode: " + var + "\n";
  // ret += "int argc;\n";
  // ret += "char *argv[];\n";

  m_argv->GetDeclCode("argv");
  m_argc->GetDeclCode("argc");
  return ret;
}
std::string ArgCVType::GetInputCode(std::string var) {
  std::string ret;
  ret += "// ArgCVType::GetInputCode: " + var + "(var ignored)" + "\n";
  ret += m_argv->GetInputCode("argv");
  ret += m_argc->GetInputCode("argc");
  return ret;
}
std::string ArgCVType::GetOutputCode(std::string var) {
  std::string ret;
  ret += "// ArgCVType::GetOutputCode: " + var + "\n";
  ret += m_argv->GetOutputCode("argv");
  ret += m_argc->GetOutputCode("argc");
  return ret;
}
InputSpec* ArgCVType::GenerateInput() {
  // std::vector<char> m_bools;
  // std::vector<char> m_named_args;
  ArgCVInputSpec *spec = new ArgCVInputSpec();
  // EXE name
  static int max_argv0_length = HeliumOptions::Instance()->GetInt("max-argv0-length");
  std::string argv0 = utils::rand_str(utils::rand_int(0, max_argv0_length));
  spec->SetArgv0(argv0);
  // bools
  for (char c : m_bools) {
    spec->AddBool(std::to_string(c), utils::rand_bool());
  }
  // named_args
  static Type *char_type = TypeFactory::CreateType("char*");
  for (char c : m_named_args) {
    InputSpec *tmp_spec = char_type->GenerateInput();
    spec->AddNamedArg(std::to_string(c), tmp_spec);
  }
  // other args
  int other_arg_count = utils::rand_int(0, 3);
  for (int i=0;i<other_arg_count;i++) {
    InputSpec *tmp_spec = char_type->GenerateInput();
    spec->AddArg(tmp_spec);
  }
  return spec;
}



/**
 * Input Spec
 */




std::string ArgCVInputSpec::GetRaw() {
  std::string ret;
  std::vector<std::string> list;
  list.push_back(m_argv0);
  for (auto m : m_bools) {
    std::string name = m.first;
    bool b = m.second;
    if (b) {
      list.push_back("-" + name);
    }
  }
  for (auto m : m_named_args) {
    std::string name = m.first;
    InputSpec *spec = m.second;
    if (spec) {
      list.push_back("-" + name);
      list.push_back(spec->GetRaw());
    }
  }
  for (InputSpec *spec : m_args) {
    if (spec) {
      list.push_back(spec->GetRaw());
    }
  }

  // adding argc
  int count = list.size() + 1;
  list.push_back(std::to_string(count));
  std::string joined = boost::algorithm::join(list, " ");
  ret += joined;
  return ret;
}


std::string ArgCVInputSpec::GetSpec() {
  std::string ret;
  ret += "{";
  std::vector<std::string> list;
  list.push_back("argv_0: " + m_argv0);
  for (auto m : m_bools) {
    std::string c = m.first;
    bool b = m.second;
    list.push_back("argv_" + c + ": " + (b?"true":"false"));
  }
  for (auto m : m_named_args) {
    std::string name = m.first;
    InputSpec *spec = m.second;
    if (spec) {
      list.push_back("argv_" + name + ": " + spec->GetSpec());
    }
  }
  for (int i=0;i<(int)m_args.size();i++) {
    InputSpec *spec = m_args[i];
    if (spec) {
      list.push_back("argv_" + std::to_string(i+1) + ": " + m_args[i]->GetSpec());
    }
  }
  std::string joined = boost::algorithm::join(list, ", ");
  ret += joined;
  ret += "}";
  return ret;
}
