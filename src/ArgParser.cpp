#include <ArgParser.hpp>
#include <iostream>

ArgParser::ArgParser(int argc, char** argv)
{
  po::options_description options("Arguments");
  options.add_options()
    ("help", "produce help message")
    ("log-level", po::value<int>()->default_value(5), "log level");
  po::options_description cmd_utils("Utils");
  cmd_utils.add_options()
    ("split", "split declaration")
    ("remove-comment", "remove comment")
    ("ctags", "create ctags");
  po::options_description experiments("Experiments");
  experiments.add_options()
    ("build-rate", "run build rate experiment")
    ("equivalence", "run equivalence checking experiment")
    ("change", "run change experiment")
    ("config", po::value<std::string>(), "config file");
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("folder", "project folder");
  // positional options
  po::positional_options_description positional;
  positional.add("folder", 1);

  // Further group
  m_cmdline_options.add(options).add(hidden).add(cmd_utils).add(experiments);
  m_help_options.add(options).add(cmd_utils).add(experiments);

  po::store(po::command_line_parser(argc, argv).
    options(m_cmdline_options).positional(positional).run(), m_vm);
  po::notify(m_vm);

  if (!validate()) {
    PrintHelp();
    exit(1);
  }
}

ArgParser::~ArgParser() {}

void ArgParser::PrintHelp() {
  std::cout<< "Usage: helium [options] <folder>" <<std::endl;
  std::cout<< m_help_options << std::endl;
}

bool ArgParser::Has(std::string name) {
  if (m_vm.count(name)) {
    return true;
  } else {
    return false;
  }
}
std::string ArgParser::GetString(std::string name) {
  if (Has(name)) {
    return m_vm[name].as<std::string>();
  } else {
    return "";
  }
}

bool ArgParser::validate() {
  if (Has("folder")) return true;
  else return false;
}

bool ArgParser::HasCmdUtils() {
  if (Has("split") || Has("remove-comment") || Has("ctags")) {
    return true;
  } else {
    return false;
  }
}
