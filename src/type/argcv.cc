#include "type.h"
#include "type_common.h"
#include "type_helper.h"

using namespace type;



std::vector<std::pair<TestInput*, TestInput*> > ArgCV::GetTestInputSpec(int number) {
    std::vector<std::pair<TestInput*, TestInput*> > ret;
    for (int i=0;i<number;i++) {
      ret.push_back(GetTestInputSpec());
    }
    return ret;
}

std::pair<TestInput*, TestInput*> ArgCV::GetTestInputSpec() {
  print_trace("ArgCV::GetTestInputSpec()");
  ArgVTestInput *argv_input = new ArgVTestInput(NULL, "argv");
  // according to spec, generate the raw
  std::vector<std::string> components;
  std::string argv_spec;
  // FIXME this should also be a random one!
  // and I should also treat it special because the name of the command should not be always arbitrary as its parameter

  int argv0_length = utils::rand_int(1, Config::Instance()->GetInt("max-argv0-length"));
  std::string str = utils::rand_str(utils::rand_int(1, argv0_length));
  components.push_back(str);
  argv_spec += "Id_strlen(argv:r0) = " + std::to_string(str.length()) + "\n";
  
  // components.push_back("helium_program");
  for (char c : m_bool_opt) {
    if (utils::rand_bool()) {
      components.push_back("-" + std::string(1, c));
      argv_spec += "Ix_argv:" + std::string(1, c) + " = true\n";
    } else {
      argv_spec += "Ix_argv:" + std::string(1, c) + " = false\n";
    }
  }
  for (char c : m_str_opt) {
    if (utils::rand_bool()) {
      components.push_back("-" + std::string(1, c));
      // the string for the argument
      int size = utils::rand_int(1, Config::Instance()->GetInt("max-strlen"));
      std::string str = utils::rand_str(utils::rand_int(1, size));
      components.push_back(str);
      argv_spec += "Ix_argv:" + std::string(1, c) + " = true\n";
      argv_spec += "Id_strlen(argv:" + std::string(1, c) + ") = " + std::to_string(str.length())+ "\n";
    }
  }
  // add 0-3 more random staff
  int random_staff = utils::rand_int(0, 3);
  for (int i=0;i<random_staff;i++) {
    int size = utils::rand_int(1, Config::Instance()->GetInt("max-strlen"));
    std::string str = utils::rand_str(utils::rand_int(1, size));
    components.push_back(str);
    // i+1 because argv[0] is special as argv:r0
    argv_spec += "Id_strlen(argv:r" + std::to_string(i+1) + ") = " + std::to_string(str.length()) + "\n";
  }
  std::string raw;
  raw += std::to_string(components.size()) + " ";
  for (std::string &s : components) {
    raw += std::to_string(s.length()+1) + " ";
    raw += s + " ";
  }
  TestInput *argc_input = new TestInput(NULL, "argc");
  int size = components.size();
  argc_input->SetRaw(std::to_string(size));
  argv_input->SetRaw(raw);
  argv_input->SetSpec(argv_spec);
  return {argc_input, argv_input};
}

/**
 * Disable because the config file seems not loaded correctly, for max-strlen
 */
TEST(TypeTestCase, DISABLED_ArgCVTest) {
  ArgCV argcv;
  argcv.SetOpt("achtvf:");
  std::pair<TestInput*, TestInput*> inputs = argcv.GetTestInputSpec();
  // inputs.first->ToString();
  // inputs.first->GetRaw();
  // inputs.second->ToString();
  // inputs.second->GetRaw();

  std::cout << inputs.first->ToString() << "\n";
  std::cout << inputs.first->GetRaw() << "\n";
  std::cout << inputs.second->ToString() << "\n";
  std::cout << inputs.second->GetRaw() << "\n";
}

std::string ArgVTestInput::dump() {
  return "This is argcv!";
}

std::string ArgVTestInput::ToString() {
  // remember to set spec!
  return m_spec;
}
