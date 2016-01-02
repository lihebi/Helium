#include "Helium.hpp"
#include "segment/Segment.hpp"
#include "Reader.hpp"
#include "Builder.hpp"
#include "Tester.hpp"
#include "Analyzer.hpp"

#include "util/ThreadUtil.hpp"
#include "util/FileUtil.hpp"
#include "ArgParser.hpp"
#include "Config.hpp"
#include "resolver/Ctags.hpp"
#include "resolver/SystemResolver.hpp"
#include "resolver/HeaderSorter.hpp"

#include "Global.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <string>

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
    Ctags::Instance()->Load(m_folder + "/tags");
  } else {
    Ctags::Instance()->Load(tagfile);
  }

  /* load system tag file */
  SystemResolver::Instance()->Load(helium_home + "systype.tags");
  HeaderSorter::Instance()->Load(m_folder);

  /* load config */
  Config::Instance()->Load(helium_home+"/helium.xml");
  
  std::string output_folder = Config::Instance()->GetOutputFolder();
  /* prepare folder */
  FileUtil::RemoveFolder(output_folder);
  FileUtil::CreateFolder(output_folder);


  /* get files in target folder */
  FileUtil::GetFilesByExtension(m_folder, m_files, "c");
}
Helium::~Helium() {}

void
Helium::Run() {
  for (auto it=m_files.begin();it!=m_files.end();it++) {
    global_file_error_number = 0;
    std::shared_ptr<Reader> reader = std::make_shared<Reader>(*it);
    reader->Read();
  }
}


