#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>

#include "Helium.hpp"
#include "Config.hpp"
#include "cmd/CommentRemover.hpp"
#include "cmd/Splitter.hpp"
#include "ArgParser.hpp"

#include "resolver/Ctags.hpp"

#include "util/FileUtil.hpp"
#include "resolver/SystemResolver.hpp"
#include "resolver/HeaderSorter.hpp"
#include "cmd/CondComp.hpp"

namespace fs = boost::filesystem;

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

int main(int argc, char* argv[]) {
  const char *helium_home_env = std::getenv("HELIUM_HOME");
  if(!helium_home_env) {
    std::cerr<<"Please set env variable HELIUM_HOME"<<std::endl;
    return 1;
  }
  fs::path helium_home(helium_home_env);
  ArgParser *arg_parser = new ArgParser(argc, argv);
  std::string folder = arg_parser->GetString("folder");
  while (folder.back() == '/') folder.pop_back();
  fs::path config_file;
  if (arg_parser->Has("build-rate")) {
    config_file =  helium_home / "buildrate.xml";
  } else if (arg_parser->Has("equivalence")) {
    config_file = helium_home / "equivalence.xml";
  } else if (arg_parser->Has("change")) {
    config_file = helium_home / "change.xml";
  } else if (arg_parser->Has("config")) {
    config_file = arg_parser->GetString("config");
  } else {
    config_file = helium_home / "helium.xml";
  }
  Config *config = Config::Instance();
  // may exit when the config file doesn't exist
  config->Load(config_file.string());
  // utils
  if (arg_parser->HasCmdUtils()) {
    if (arg_parser->Has("split")) {
      Splitter(folder).Run();
    } else if (arg_parser->Has("remove-comment")) {
      CommentRemover(folder).Run();
    } else if (arg_parser->Has("ctags")) {
      create_ctags(folder);
    } else if (arg_parser->Has("cond-comp")) {
      CondComp(folder).Run();
    } else if (arg_parser->Has("pre")) {
      CommentRemover(folder).Run();
      Splitter(folder).Run();
      CondComp(folder).Run();
      create_ctags(folder);
    }
  } else {
    // main
    // may exit if the tag file cannot load
    // TODO tags file should appear in another location rather than the code base itself,
    // because the code base is stored read-only
    std::string tagfile = arg_parser->GetString("tagfile");
    if (tagfile.empty()) {
      Ctags::Instance()->Load(folder + "/tags");
    } else {
      Ctags::Instance()->Load(tagfile);
    }
    // may exit if the tag file cannot load
    SystemResolver::Instance()->Load((helium_home / "systype.tags").string());
    HeaderSorter::Instance()->Load(folder);
    std::string output_folder = config->GetOutputFolder();
    FileUtil::RemoveFolder(output_folder);
    FileUtil::CreateFolder(output_folder);
    new Helium(folder);
  }
  return 0;
}
