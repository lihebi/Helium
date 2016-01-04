#include "helium.h"
#include <string>
#include <iostream>
#include "arg_parser.h"
#include "snippet.h"
#include "resolver.h"
#include "config.h"
#include "utils.h"
#include "reader.h"

using namespace utils;

/**
 * read env variable HELIUM_HOME
 * @return std::string home folder without trailing '/'
 */
std::string
load_helium_home() {
  /* read HELIUM_HOME env variable */
  const char *helium_home_env = std::getenv("HELIUM_HOME");
  if(!helium_home_env) {
    std::cerr<<"Please set env variable HELIUM_HOME"<<std::endl;
    exit(1);
  }
  std::string helium_home = helium_home_env;
  while (helium_home.back() == '/') helium_home.pop_back();
  return helium_home;
}

static void
create_ctags(const std::string& folder) {
  std::string cmd = "ctags -f ";
  // cmd += folder + "/tags";
  cmd += "tags"; // current folder
  cmd += " --languages=c,c++ -n --c-kinds=+x --exclude=heium_result -R ";
  cmd += folder;
  std::cout<< "create_ctags: " << cmd <<std::endl;
  std::system(cmd.c_str());
}


Helium::Helium(int argc, char** argv) {
  /* load HELIUM_HOME */
  std::string helium_home = load_helium_home();
  /* parse arguments */
  ArgParser* args = new ArgParser(argc, argv);
  // target folder
  m_folder = args->GetString("folder");
  while (m_folder.back() == '/') m_folder.pop_back();
  /* load tag file */
  std::string tagfile = args->GetString("tagfile");
  if (tagfile.empty()) {
    ctags_load(m_folder + "/tags");
  } else {
    ctags_load(tagfile);
  }

  /* load system tag file */
  SystemResolver::Instance()->Load(helium_home + "systype.tags");
  HeaderSorter::Instance()->Load(m_folder);

  /* load config */
  Config::Instance()->ParseFile(helium_home+"/helium.conf");
  
  std::string output_folder = Config::Instance()->GetString("output_folder");
  /* prepare folder */
  remove_folder(output_folder);
  create_folder(output_folder);


  /* get files in target folder */
  get_files_by_extension(m_folder, m_files, "c");
}
Helium::~Helium() {}

void
Helium::Run() {
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    Reader(*it).Read();
  }
}


